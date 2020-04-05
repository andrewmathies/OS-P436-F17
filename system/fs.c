#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

/* YOUR CODE GOES HERE */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void
fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}


int fs_open(char *filename, int flags) {
  

  return SYSERR;
}

int fs_close(int fd) {
  return SYSERR;
}

int fs_create(char *filename, int mode) {
  
  // search root directory for file

  // if it exists return

  // else make inode for file with blocksize of 0
  int fd = next_open_fd++;

  struct inode *inode = &(oft[fd].in);
  int inode_num = fsd.inodes_used++;

  inode->id = inode_num;
  inode->type = INODE_TYPE_FILE;
  inode->nlink = 1;
  inode->device = 0;
  inode->size = 0;

  // need to set up rest of filetable struct oft[fd]
  fs_put_inode_by_num(0, inode_num, inode);

  oft[fd].state = FSTATE_OPEN;
  //oft[fd].fileptr = filename; ?
  
  return fd;
}

int fs_seek(int fd, int offset) {
  return SYSERR;
}

int fs_read(int fd, void *buf, int nbytes) {
  struct filetable fileStuff = oft[fd];
  
  if (fileStuff.state == FSTATE_CLOSED) {
    printf("error, attempted to write to closed file\n");
    return 0;
  }

  int blocksToRead;
  if (nbytes % fsd.blocksz == 0)
    blocksToRead = nbytes / fsd.blocksz;
  else
    blocksToRead = nbytes / fsd.blocksz + 1;

  struct inode inode = fileStuff.in;
  int blocks[INODEBLOCKS];
  memcpy(blocks, inode.blocks, sizeof(blocks));

  void *cache = getmem(fsd.blocksz);
  int remainingDataSize = sizeof(buf), index;

  for (index = 0; index < blocksToRead; index++) {
    if (remainingDataSize - fsd.blocksz < 0) {
      // don't need to read the entire next block
      bs_bread(0, blocks[index], 0, buf + (index * fsd.blocksz), remainingDataSize);
    }  else {
      // read the whole block
      bs_bread(0, blocks[index], 0, buf + (index * fsd.blocksz), fsd.blocksz);
    }
    
    remainingDataSize -= fsd.blocksz;
  }
  
  return OK;
}

int fs_write(int fd, void *buf, int nbytes) {
  struct filetable fileStuff = oft[fd];
  
  if (fileStuff.state == FSTATE_CLOSED) {
    printf("error, attempted to write to closed file\n");
    return 0;
  }
  
  int blocksToAllocate;

  if (nbytes % fsd.blocksz == 0)
    blocksToAllocate = nbytes / fsd.blocksz;
  else
    blocksToAllocate = nbytes / fsd.blocksz + 1;

  int byte, bit, index;
  int blocks[INODEBLOCKS];
  memset(&blocks, 0, sizeof(blocks));

  for (byte = 0; byte < fsd.freemaskbytes; byte++) {
    for(bit = 0; bit < 8; bit++) {
      if (!fs_getmaskbit(byte * 8 + bit)) {
	//this block is free!
	blocks[index++] = byte * 8 + bit;
	fs_setmaskbit(byte * 8 + bit);
	
	if (index == blocksToAllocate) {
	  // we're done searching for free blocks!
	  byte = fsd.freemaskbytes;
	  break;
	}
      }
    }
  }

  struct inode inode = fileStuff.in;
  inode.size = blocksToAllocate;
  memcpy(inode.blocks, blocks, sizeof(blocks));

  void *cache = getmem(fsd.blocksz);
  int remainingDataSize = sizeof(buf);

  for (index = 0; index < blocksToAllocate; index++) {
    memcpy(cache, buf + (index * fsd.blocksz), fsd.blocksz);
     
    if (remainingDataSize - fsd.blocksz < 0) {
      // don't have enough data left to fill up entire block
      bs_bwrite(0, blocks[index], 0, cache, remainingDataSize);
    }  else {
      // fill up the whole block
      bs_bwrite(0, blocks[index], 0, cache, fsd.blocksz);
    }
    
    remainingDataSize -= fsd.blocksz;
  }
  
  // successfully wrote file!
  return nbytes;
}

#endif /* FS */
