
static unsigned long int next = 1;

void rand_delay(int ms_max) {
  next = next * 1103515245 + 12345;
  unsigned int rand = (unsigned int)(next / 65536) % ms_max + 1;
  //printf("Sleeptime: %d\n", rand);
  sleepms(rand);
}

/*
don't need this? apparently this function is already implemented somewhere 
void srand( unsigned int seed )
{
  next = seed;
}
*/
