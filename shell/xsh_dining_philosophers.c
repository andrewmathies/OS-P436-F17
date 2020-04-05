#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>

void philosopher(int philosopherNumber);

int philosophers, cycles, max_delay, i;
sid32 footman, forks[100];
pid32 pids[100];

shellcmd xsh_dining_philosophers(int nargs, char *args[]) {
  if (nargs != 4) {
    printf("Incorrect input!\n");
    return 1;
  }

  philosophers = atoi(args[1]);
  cycles = atoi(args[2]);
  max_delay = atoi(args[3]);

  footman = semcreate(philosophers);

  for (i = 0; i < philosophers; i++)
    forks[i] = semcreate(1);

  char buff[14];
  for (i = 0; i < philosophers; i++) {
    sprintf(buff, "philosopher", i);
    pids[i] = resume(create(philosopher, 512, 20, buff, 1, i));
  }

  return 0;
}

int left(int i) {
  return i;
}

int right(int i) {
  return (i + 1) % philosophers;
}

void put_forks(int philosopherNumber) {
  signal(forks[right(philosopherNumber)]);
  signal(forks[left(philosopherNumber)]);
  signal(footman);
}

void get_forks(int philosopherNumber) {
  wait(footman);
  wait(forks[right(philosopherNumber)]);
  wait(forks[left(philosopherNumber)]);
}

void philosopher(int philosopherNumber) {
  int cycle;
  
  for(cycle = 0; cycle < cycles; cycle++) {
    rand_delay(max_delay);
    printf("Philosopher %d thinking cycle %d\n", philosopherNumber, cycle);
    get_forks(philosopherNumber);
    rand_delay(max_delay);
    printf("Philosopher %d eating cycle %d\n", philosopherNumber, cycle);
    put_forks(philosopherNumber);
  }
  
  kill(pids[philosopherNumber]);
}
