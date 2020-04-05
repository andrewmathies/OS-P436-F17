#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int fd[2], nbytes, status;
  pid_t amIAChild;
  char readbuffer[20];

  if (argc > 2) {
    printf("Too many arguements.\n");
    return 1;
  } else if (argc == 1) {
    printf("No arguement given.\n");
    return 1;
  }

  pipe(fd);
  amIAChild = fork();

  if (amIAChild == 0) {
    //yes
    close(fd[1]);
    nbytes = read(fd[0], readbuffer, 20);
    printf("Child PID = %d\n", getpid());
    if (strcmp(readbuffer, "/bin/echo") == 0) {
      execl(readbuffer, readbuffer, "Hello World!", (char *) NULL);
    } else {
      execl(readbuffer, readbuffer, (char *) NULL);
    }
    printf("shouldn't get here \n");
  } else {
    // no
    close(fd[0]);
    write(fd[1], argv[1], strlen(argv[1]));
    printf("Parent PID = %d\n", getpid());
    wait(&status);
  }
  return 0;
}
