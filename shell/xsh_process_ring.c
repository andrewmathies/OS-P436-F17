#include <xinu.h>
#include <stdio.h>

shellcmd xsh_process_ring(int nargs, char *args[]) {
  /*  if (nargs == 0) {
    printf("No arguements given.\n");
    return 1;
  }
  int32 p = 2; /* specified default 
  char *endp;
  int32 i;
  for(i = 1; i < nargs; i++){
    if (0 == strncmp("-p", args[i], 3)){
      /* parse numeric argument to -p 
      if (! (i + 1 < nargs)) {
        /* No argument after flag 
        print_usage();
        printf("-p flag expected an argument\n");
        return SHELL_ERROR;
      }
      p = strtol(args[i + 1], &endp, 10);
      if (endp == args[i + 1]) {
        /* The end pointer hasn't advanced,
           No number was parsed.
        print_usage();
        printf("-p flag expected an integer\n");
        return SHELL_ERROR;
      } else if (*endp != '\0'){
	/* There was trailing garbage in the string that
	   wasn't converted to an integer. 
        print_usage();
        printf("-p recieved invalid integer\n");
        return SHELL_ERROR;
      } else if (!(0 <= p && p <= 64)) {
        /* The number is out of range 
        print_usage();
        printf("-p flag expected a number between 0 - 64\n");
        return SHELL_ERROR;
      }
      /* Skip pass the numeric argument,
         that was successfully parsed 
      i += 1; 
    } else /* if (...)  { 
      // Handle other cases
    }
  }
  printf("argument  is %d\n", p); */
  return SHELL_OK;
}
