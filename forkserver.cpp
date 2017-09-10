/*
  Lauch a forkserver to speed up fuzzing,
  I steal the idea and most of code from AFL.
 */
#include "config.h"
#include <sys/shm.h>
#include <stack>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>

#define DEFER_ENV_VAR "DEFER_FORKSRV"

extern "C" {

__attribute__((noinline)) void AFLStartStub() { return; }

void AFLStartForkServer() {

  static u8 tmp[4];
  s32 child_pid = 0;

  /* Phone home and tell the parent that we're OK. If parent isn't there,
     assume we're not running in forkserver mode and just execute program. */
  if (write(FORKSRV_FD + 1, tmp, 4) != 4) {
    fprintf(stderr, "can't find forksrv_fd! \n");
    return;
  }

  while (1) {

    u32 was_killed;
    s32 status;
    
    /* Wait for parent by reading from the pipe. Abort if read fails. */
    if (read(FORKSRV_FD, &was_killed, 4) != 4) {
      fprintf(stderr, "exit forkserver loop\n");
      _exit(0);
    }

    child_pid = fork();
    if (child_pid < 0) _exit(1);

    if (!child_pid) { // child
      //fprintf(stderr, "child..\n");
      close(FORKSRV_FD);
      close(FORKSRV_FD + 1);
      return;
    }

    //fprintf(stderr, "parent..\n");

    if (write(FORKSRV_FD + 1, &child_pid, 4) != 4) _exit(1);

    if (waitpid(child_pid, &status, 0) < 0)
      _exit(1);

    //fprintf(stderr, "status in forkserver: %d \n", status);

    if (write(FORKSRV_FD + 1, &status, 4) != 4) _exit(1);

  }

}

}

__attribute__ ((constructor)) void __forkserver_init() {

  //fprintf(stderr, "has fs..\n");

  if (getenv(DEFER_ENV_VAR)) return;

  // There is a bug in pin2.14 intel64 version Pin,
  // the PIN_APP_LD_PRELOAD will run in both before pin and the target program
  // So will not call it here while using Pin
  if (!getenv(FORKSRV_AUTO_INIT)) {
    AFLStartStub();
    return;
  }

  // In intel64 architecture, there will launch two process in PIN
  // and both will call constructor function.
  // We should avoid to call them twice
  AFLStartForkServer();

}
