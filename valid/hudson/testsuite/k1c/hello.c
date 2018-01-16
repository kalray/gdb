#include <stdio.h>



int main(int argc, char **argv) {
  int i;

  for(i=0; i< argc; i++) {
#if 0
    printf("Arg %d: Hello world '%s'!\n", i, argv[i]);
#else
    printf("Hello world '%s'!\n", argv[i]);
#endif
  }
  if(argc == 1) {
    return -1;
  }
  return 0;
}
