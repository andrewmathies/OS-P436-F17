#include <xinu.h>

qid16 readylist;

void age_processes() {
  /* 
 int pid;
  struct qentry *queuetabptr;
  struct procent *procptr;

  // start at 1 to skip null process, which we don't want to age
  for (pid = 1; pid < NPROC; pid++) {
    if (isbadpid(pid))
      pid++;

    queuetabptr = &queuetab[pid];

    if (pid == readylist) {
      procptr = &proctab[pid];

      if (procptr->prstate != PR_CURR)
	procptr->prprio++;
    }
  }
  */
}


