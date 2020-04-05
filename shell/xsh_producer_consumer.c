
#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <future.h>

void myProducer();
void myConsumer();
uint future_prod(future_t* fut, int n);
uint future_cons(future_t* fut);


sid32 mutex, items, spaces;
int max_delay, count = 0, bytesToSend;
pid32 producerPID, consumerPID;

shellcmd xsh_producer_consumer(int nargs, char *args[]) {
  /* this isn't working, so I'm commenting it out
  char check[2];
  strcpy(check, "-f");
  */
  if (nargs == 2) {    // && strcmp(args[1], check)) {
    // this is for assignment 6
    future_t* f_exclusive,
      * f_shared,
      * f_queue;
 
    f_exclusive = future_alloc(FUTURE_EXCLUSIVE);
    f_shared    = future_alloc(FUTURE_SHARED);
    f_queue     = future_alloc(FUTURE_QUEUE);
 
    // Test FUTURE_EXCLUSIVE
    resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
    resume( create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, 1) );
    /*
    // Test FUTURE_SHARED
    resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
    resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
    resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) ); 
    resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
    resume( create(future_prod, 1024, 20, "fprod2", 2, f_shared, 2) );

    // Test FUTURE_QUEUE
    resume( create(future_cons, 1024, 20, "fcons6", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons8", 1, f_queue) );
    resume( create(future_cons, 1024, 20, "fcons9", 1, f_queue) );
    resume( create(future_prod, 1024, 20, "fprod3", 2, f_queue, 3) );
    resume( create(future_prod, 1024, 20, "fprod4", 2, f_queue, 4) );
    resume( create(future_prod, 1024, 20, "fprod5", 2, f_queue, 5) );
    resume( create(future_prod, 1024, 20, "fprod6", 2, f_queue, 6) );
    */
    return 0;
  }

  if (nargs != 4) {
    printf("Incorrect input, should be:\n1st arguement: buffer size in bytes \n2nd arguement: bytes to send \n3rd arguement: max delay\n");
    return 1;
  }



  /* when consumer removes an item it hsould signal spaces, when a producer arrives it
     should decrement spaces, at which point it might block until next consumer signals */
  mutex = semcreate(1);
  items = semcreate(0);
  spaces = semcreate((int) args[1]);

  int size = atoi(args[1]);
  max_delay = atoi(args[3]);
  bytesToSend = atoi(args[2]);

  consumerPID = resume(create(myConsumer, size, 60, "consumer", 0));
  producerPID = resume(create(myProducer, size, 60, "producer", 0));

  return 0;
}

void myProducer() {
  while (bytesToSend != count) {
    // make thing
    rand_delay(max_delay);
    printf("Producer: 0x%x\n", count);

    wait(spaces);
    wait(mutex);
    count++;
    if (count == 255) {
      count = 0;
      bytesToSend -= 256;
    }
    signal(mutex);
    signal(items);
  }
  //printf("producer killed itself\n");
  kill(producerPID);
}

void myConsumer() {
  //int value = 0;

  while (bytesToSend != count) {
    wait(items);
    wait(mutex);
    //value = count;
    // do I even need to do anything here?
    signal(mutex);
    signal(spaces);

    // process thing
    rand_delay(max_delay);
    printf("Consumer: 0x%x\n", count - 1);
  }
  //printf("consumer killed itself\n");
  count = 0;
  kill(consumerPID);
}

uint future_prod(future_t* fut,int n) {
  future_set(fut, n);
  printf("Produced %d\n",n);
  return OK;
}

uint future_cons(future_t* fut) {
  int i, status;
  //printf("trying to get\n");
  status = (int)future_get(fut, &i);
  //printf("get worked\n");
  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }
  printf("Consumed %d\n", i);
  return OK;
}
