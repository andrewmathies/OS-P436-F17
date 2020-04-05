#include <xinu.h>
#include <future.h>


future_t* future_alloc(future_mode_t mode) {
  //printf("before getmem\n");
  future_t* future = (future_t *) getmem(sizeof(future_t));
  //printf("after getmem\n");
  
  future->state = FUTURE_EMPTY;
  future->mode = mode;

  if (mode == FUTURE_QUEUE)
    future->set_queue = newqueue();
  if (mode != FUTURE_EXCLUSIVE)
    future->get_queue = newqueue();

  return future;
}

syscall future_free(future_t* f) {
  intmask mask;

  mask = disable();

  resched_cntl(DEFER_START);
  if (f->mode == FUTURE_EXCLUSIVE)
    ready(f->pid);
  else {
    qid16 q = f->get_queue;
      
    while (!isempty(q))
      ready(dequeue(q));
  }

  if (f->mode == FUTURE_QUEUE) {
    qid16 q = f->set_queue;
      
    while (!isempty(q))
      ready(dequeue(q));
  }
  resched_cntl(DEFER_STOP);

  freemem((char *) f, sizeof(f)); // idk if this should happen before or after defer is turned off, I
  restore(mask);                  // don't think it matters

  return OK;
}

syscall future_get(future_t* f, int* value) {
  intmask mask;
  struct procent *prptr;

  mask = disable();

  if (f->mode == FUTURE_EXCLUSIVE) {
    if (f->state == FUTURE_EMPTY) { //future isn't set yet, so make this process wait
      f->state = FUTURE_WAITING;
      f->pid = currpid;

      prptr = &proctab[currpid];
      prptr->prstate = PR_WAIT;
      resched();
    } else if (f->state == FUTURE_WAITING) {
      printf("this is bad\n");
      return SYSERR; // should not be possible
    } else { // state is ready so just give it the value
      *value = f->value;
    }
  } 
  else if (f->mode == FUTURE_SHARED) {
    if (f->state != FUTURE_READY) {
      f->state = FUTURE_WAITING;
      enqueue(currpid, f->get_queue);

      prptr = &proctab[currpid];
      prptr->prstate = PR_WAIT;
      resched();
    } else {
      *value = f->value;
    }
  }
  else { // we're in queue mode!
    if (f->state != FUTURE_READY) {
      f->state = FUTURE_WAITING;
      enqueue(currpid, f->get_queue);

      prptr = &proctab[currpid];
      prptr->prstate = PR_WAIT;
      resched();
    } else { // we're in ready state
      resume(dequeue(f->set_queue));  // this will resume the first thread that tried to set a value,
      *value = f->value;               // which will give us a value to return
    }
  } 

  restore(mask);
  return OK;
}

syscall future_set(future_t* f, int value){
  if (f->mode != FUTURE_QUEUE) {
    f->value = value;

    //printf("this should print\n");

    if (f->state == FUTURE_EMPTY) {
      f->state = FUTURE_READY;
    } 
    else if (f->state == FUTURE_WAITING) {
      if (f->mode == FUTURE_SHARED) {
	qid16 q = f->get_queue;
      
	while (!isempty(q))
	  resume(dequeue(q));
      } 
      else // mode is exclusive
	resume(f->pid);
    } 
    else {
      printf("this is bad 2\n");
      return SYSERR; // if future is exclusive or shared and already ready theres a problem
    }   
    return OK; 
  }

  // this part is for futures in queue mode
  if (f->state == FUTURE_READY) {
    enqueue(currpid, f->set_queue);
    struct procent *prptr = &proctab[currpid];
    prptr->prstate = PR_WAIT;
    resched();
    f->value = value;
  } 
  else if (f->state == FUTURE_WAITING) {
    f->value = value;
    resume(dequeue(f->get_queue));
  }
  else {
    f->value = value;
    f->state = FUTURE_READY;
  }
  
  return OK;
}
