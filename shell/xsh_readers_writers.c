
#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>

void reader(int myNumber);
void writer(int myNumber);

int readers = 0;
sid32 mutex, room_empty, turnstile;

int writers_input, readers_input, write_cycles, read_cycles, max_delay;

pid32 readerPIDS[100], writerPIDS[100];

shellcmd xsh_readers_writers(int nargs, char *args[]) {
  if (nargs != 6) {
    printf("Incorrect input.\n");
    return 1;
  }

  writers_input = atoi(args[1]);
  readers_input = atoi(args[2]);
  write_cycles = atoi(args[3]);
  read_cycles = atoi(args[4]);
  max_delay = atoi(args[5]);

  mutex = semcreate(1);
  room_empty = semcreate(1);
  turnstile = semcreate(1);

  int i;
  char buf[9];

  for (i = 0; i < readers_input; i++) {
    sprintf(buf, "reader", i);
    readerPIDS[i] = resume(create(reader, 512, 20, buf, 1, i));
  }
  for (i = 0; i < writers_input; i++) {
    sprintf(buf, "writer", i);
    writerPIDS[i] = resume(create(writer, 512, 20, buf, 1, i));
  }

  return 0;
}


void reader(int myNumber) {
  int cycle;

  for (cycle = 0; cycle < read_cycles; cycle++) {
    wait(turnstile);
    signal(turnstile);

    wait(mutex);
  
    readers++;
    if (readers == 1)
      wait(room_empty);

    signal(mutex);

    // critical section
    rand_delay(max_delay);
    printf("Reader %d %d\n", myNumber, cycle); 

    wait(mutex);
  
    readers--;
    if (!readers)
      signal(room_empty);

    signal(mutex);
  }
  
  kill(readerPIDS[myNumber]);
}

void writer(int myNumber) {
  int cycle;

  for (cycle = 0; cycle < write_cycles; cycle++) {
    wait(turnstile);
    wait(room_empty);
  
    // critical section
    rand_delay(max_delay);
    printf("Writer %d %d\n", myNumber, cycle);

    signal(turnstile);
    signal(room_empty);
  }

  kill(writerPIDS[myNumber]);
}
