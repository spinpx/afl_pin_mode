#include "config.h"

s32 fsrv_ctl_fd;
s32 fsrv_st_fd;

void init_forkserver(char** argv) {
  int st_pipe[2], ctl_pipe[2];

  if (pipe(st_pipe) || pipe(ctl_pipe)) {
    fprintf(stderr, "pipe() failed \n");
  }

  s32 forksrv_pid = fork();
  if (forksrv_pid < 0) fprintf(stderr, "fork() failed \n");

  if (!forksrv_pid) {

    setsid();

    if (dup2(ctl_pipe[0], FORKSRV_FD) < 0) fprintf(stderr, "dup2() failed \n");
    if (dup2(st_pipe[1], FORKSRV_FD + 1) < 0) fprintf(stderr, "dup2() failed \n");

    close(ctl_pipe[0]);
    close(ctl_pipe[1]);
    close(st_pipe[0]);
    close(st_pipe[1]);

    //setenv("LD_PRELOAD", "./forkserver.so", 0);
    char *FKSO = (char*)"./forkserver.so";
    if (!getenv("USING_PIN")) {
      setenv(PRELOAD_VAR, FKSO, 0);
      setenv(FORKSRV_AUTO_INIT, "1", 0);
      //setenv(PRELOAD_VAR, FKSO, 0);
    } else {
      setenv(PIN_PRELOAD_VAR, FKSO, 0);
    }

    fprintf(stderr, "exec target -- %s\n", argv[0]);

    execv(argv[0], argv);

    fprintf(stderr, "sth err after execv \n");

    exit(0);

  }

  close(ctl_pipe[0]);
  close(st_pipe[1]);

  fsrv_ctl_fd = ctl_pipe[1];
  fsrv_st_fd  = st_pipe[0];

  s32 status = 0;
  s32 rlen = read(fsrv_st_fd, &status, 4);

  if (rlen == 4) {
    fprintf(stderr, "All right - fork server is up.\n");
    return;
  }

  fprintf(stderr, "init fork server fail\n");

}

void run() {
  s32 res;

  s32 sth = 0;
  if ((res = write(fsrv_ctl_fd, &sth, 4)) != 4) {
    fprintf(stderr, "unable to request new preocess from fork server -- write fd !\n");
    return;
  }

  s32 child_pid;
  if ((res = read(fsrv_st_fd, &child_pid, 4)) != 4) {
    fprintf(stderr, "unable to request new preocess from fork server -- recv child_pid!\n");
    return;
  }

  if (child_pid <= 0) {
    fprintf(stderr, "fork server is misbehaving\n");
    return;
  }

  s32 status;

  if ((res = read(fsrv_st_fd, &status, 4)) != 4) {
    fprintf(stderr, "unable to communicate with fork server !\n");
    return;
  }

  //fprintf(stderr, "status is : %d \n", status);

}

int main (int argc, char** argv) {
  if (argc < 2) {
    return 0;
  }

  init_forkserver(&argv[1]);

  for (int i = 0; i < 10; i++) {
    run();
  }

  int sth = 0;
  // break the loop
  if (write(fsrv_ctl_fd, &sth, 2) != 2) {
    fprintf(stderr, "fail to break loop\n");
  }

}
