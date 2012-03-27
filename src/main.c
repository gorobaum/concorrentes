
#include <stdio.h>
#include <stdlib.h>

#include "race.h"

int
main (int argc, char **argv) {

  if (argc < 2) {
    puts("-.-");
    return EXIT_FAILURE;
  }
  
  if (RACEload(argv[1]))
    return EXIT_FAILURE;
  
  RACEdisplay_info();

  if (RACErun())
    return EXIT_FAILURE;

  RACEcleanup();

  return EXIT_SUCCESS;

}

