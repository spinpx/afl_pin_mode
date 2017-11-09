#include "config.h"
#include "pin_config.h"


int main (int argc, char** argv) {

  setenv(PIN_PRELOAD_VAR, PIN_APP_LD_PRELOAD, 0);
  // TO bypass check the binary in AFL
  setenv("MY_NOISE", SHM_ENV_VAR, 0);

  char* new_args[50];
  int i = 0;
  new_args[i++] = (char*)PIN_BIN;
  new_args[i++] = (char*)"-t";
  new_args[i++] = (char*)PIN_TOOL;
  new_args[i++] = (char*)"--";

  for (int j =1; j < argc; j++) {
    new_args[i++] = argv[j];
  }
  new_args[i] = NULL;


  // i = 0;
  // while (1) {
  //   if (new_args[i] == NULL) break;
  //   printf("%s\n", new_args[i]);
  //   i++;
  // }

  execv(new_args[0], new_args);
}
