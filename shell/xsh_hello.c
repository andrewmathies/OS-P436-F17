
#include <xinu.h>

shellcmd xsh_hello (int nargs, char *args[]) {
  if (nargs > 2) {
    printf("Too many arguements entered.\n");
    return 1;
  } else if (nargs == 1) {
    printf("No arguements entered\n.");
    return 1;
  }

  printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
  return 0;
}
