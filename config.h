#ifndef PIN_MODE_CONFIG_H
#define PIN_MODE_CONFIG_H

#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_SIZE_WIDTH 16
#define MAP_SIZE (1 << MAP_SIZE_WIDTH)
#define FORKSRV_FD          198
#define SHM_ENV_VAR "__AFL_SHM_ID"
#define PRELOAD_VAR "LD_PRELOAD"
#define PIN_PRELOAD_VAR "PIN_APP_LD_PRELOAD"
#define FORKSRV_AUTO_INIT "FORKSRV_AUTO_INIT"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#endif
