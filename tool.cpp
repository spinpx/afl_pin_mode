#include "config.h"
#include "pin.H"

#include <cstring>
#include <iostream>
#include <stack>
#include <sys/shm.h>
#include <time.h>

#define ADDR_PREFIX(_addr) ((_addr << 1) % MAP_SIZE)
#define ADDR_GET(_addr) (_addr % MAP_SIZE)

u8 branch_map[MAP_SIZE];
u8 *path_shm = branch_map;

bool enable_fork = false;
static ADDRINT fork_func = 0;
RTN fork_point;

// PIN_FAST_ANALYSIS_CALL
VOID TrackCondBranch(u32 prefix, bool taken) {
  path_shm[(prefix) | (u32)taken]++;
}
VOID TrackIndBranch(u32 from, u32 to) { path_shm[from ^ to]++; }

// In this function, we call the "AFLStartForkServer" fucntion in
// forkserver.so
VOID StartFork(CONTEXT *ctxt, THREADID tid) {
  PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, AFUNPTR(fork_func),
                              NULL, PIN_PARG_END());
}

static void DTearly() { PIN_Detach(); }

VOID ImageLoad(IMG img, VOID *v) {
  std::cerr << "IMG: " << IMG_Name(img) << std::endl;

  if (IMG_IsMainExecutable(img)) {

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
      // std::cerr << "  SEC: " << SEC_Name(sec) << std::endl;

      if (SEC_Name(sec) == ".text") {
        fork_point = RTN_FindByName(img, "main");

        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
          RTN_Open(rtn);
          for (INS ins = RTN_InsHead(rtn); INS_Valid(ins);
               ins = INS_Next(ins)) {
            // * Branch-based:
            // Entry id: ( branch address[n-1 bits] | taken[1 bit] )
            // DirectBranch is fixed, so we must ensure they are conditional,
            if (INS_Category(ins) == XED_CATEGORY_COND_BR) {

              ADDRINT addr = INS_Address(ins);
              u32 prefix = ADDR_PREFIX(addr);
              INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TrackCondBranch,
                             IARG_UINT32, prefix, IARG_BRANCH_TAKEN, IARG_END);
            }
            // While IndirectBranch is not fixed, the target address will
            // change.
            // We should consider both from and to addresses.
            // Including: Ret, Call
            else if (INS_IsIndirectBranchOrCall(ins)) {

              u32 from_addr = ADDR_PREFIX(INS_Address(ins));
              // ret or call?
              // FIXME: is ret has inst_ptr?
              INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TrackIndBranch,
                             IARG_UINT32, from_addr, IARG_INST_PTR, IARG_END);
            }
          }

          RTN_Close(rtn);
        }
      }
    }
  }

  if (IMG_Name(img).find("forkserver.so") != string::npos) {

    RTN rtn = RTN_FindByName(img, "AFLStartForkServer");
    if (RTN_Valid(rtn)) {
      fork_func = RTN_Address(rtn);
      std::cerr << "[Fork] find fork func!" << RTN_Name(rtn) << std::endl;
    } else {
      return;
    }

    if (RTN_Valid(fork_point)) {

      std::cerr << "[Fork] find fork point!" << RTN_Name(fork_point)
                << std::endl;
      // it is main function
      RTN_Open(fork_point);
      RTN_InsertCall(fork_point, IPOINT_BEFORE, (AFUNPTR)StartFork,
                     IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
      RTN_InsertCall(fork_point, IPOINT_AFTER, (AFUNPTR)DTearly, IARG_END);
      RTN_Close(fork_point);

    } else {
      // If there are none main routine, which means it has been stripped,
      // So we can't determine which address is the entrance
      fork_point = RTN_FindByName(img, "AFLStartStub");
      if (RTN_Valid(fork_point)) {
        std::cerr << "[Fork] find fork point!" << RTN_Name(fork_point)
                  << std::endl;
        RTN_Open(fork_point);
        RTN_InsertCall(fork_point, IPOINT_AFTER, (AFUNPTR)StartFork,
                       IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        RTN_Close(fork_point);
      }
    }
  }
}

void SetupShm() {

  if (char *shm_id_str = getenv(SHM_ENV_VAR)) {
    int shm_id = atoi(shm_id_str);
    // std::cerr << "shm_id: " << shm_id << std::endl;
    path_shm = reinterpret_cast<u8 *>(shmat(shm_id, NULL, 0));
    if (path_shm == reinterpret_cast<void *>(-1)) {
      std::cerr << "failed to get shm addr from shmmat()" << std::endl;
      _exit(1);
    }
  }
}

int main(int argc, char *argv[]) {

  PIN_InitSymbols();

  if (PIN_Init(argc, argv)) {
    std::cerr
        << "Sth error in PIN_Init. Plz use the right command line options."
        << std::endl;
    return -1;
  }

  SetupShm();
  IMG_AddInstrumentFunction(ImageLoad, 0);
  PIN_StartProgram();
  return 0;
}
