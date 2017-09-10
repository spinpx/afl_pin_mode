TARGET := $(shell uname -p)
ifeq ($(TARGET),x86_64)
	PIN_OBJ_DIR=obj-intel64
else
	PIN_OBJ_DIR=obj-ia32
endif
CUR_DIR = $(shell pwd)

#PIN_NAME=pin-3.4-97438-gf90d1f746-gcc-linux
PIN_NAME=pin-2.14-71313-gcc.4.4.7-linux
PIN_URL="http://software.intel.com/sites/landingpage/pintool/downloads/${PIN_NAME}.tar.gz"
PIN_TAR=${CUR_DIR}/${PIN_NAME}.tar.gz

PIN_ROOT=${CUR_DIR}/${PIN_NAME}
PIN_BIN=${PIN_ROOT}/pin
TOOL_NAME=tool.so
PIN_TOOL=${CUR_DIR}/${PIN_OBJ_DIR}/${TOOL_NAME}
PIN_TOOL_PREFIX= ${PIN_BIN} -t ${PIN_TOOL} --

define newline


endef

define CONFIG_BODY
#ifndef AFL_PIN_CONFIG_H
#define AFL_PIN_CONFIG_H
#define PIN_APP_LD_PRELOAD \"${CUR_DIR}/forkserver.so\"
#define PIN_BIN \"${PIN_BIN}\"
#define PIN_TOOL \"${PIN_TOOL}\"
#endif
endef

PIN_CONFIG_STR="$(subst $(newline),\n,${CONFIG_BODY})"
PIN_CONFIG_FILE="${CUR_DIR}/pin_config.h"

all: check test_app crash_app forkserver simulate pin_tool mini pin_run

check: | ${PIN_ROOT}
	@echo ${PIN_CONFIG_STR} > ${PIN_CONFIG_FILE}
${PIN_ROOT}:
	@echo "install pin ...."
	@wget ${PIN_URL}
	@tar -xf ${PIN_TAR}
  # write pin config file.

install_afl:
	@wget http://lcamtuf.coredump.cx/afl/releases/afl-latest.tgz
	@tar -xf afl-latest.tgz
	# TODO: Name of afl-2.51b?

forkserver: forkserver.cpp
	${CXX} -O3 -std=c++11 -Wall -shared -fPIC -o forkserver.so forkserver.cpp

simulate: simulate.cpp
	$(CXX) -O3 -std=c++11 -o simulate simulate.cpp

test_app: test.cpp
	$(CXX) -O3 -o test test.cpp

crash_app: crash.cpp
	$(CXX) -O3 -o crash crash.cpp

mini_app: mini.c
	$(CXX) -O3 -o mini mini.cpp

pin_run: pin_run.cpp
	$(CXX) -O3 -std=c++11 -o pin_run pin_run.cpp

pin_test: test_app crash_app forkserver simulate
	USING_PIN=1 ./simulate ${PIN_TOOL_PREFIX} ./test

no_pin_test: test_app crash_app forkserver simulate
	./simulate ./test

pin_tool:
	PIN_ROOT=${PIN_ROOT} make -f makefile.pin

test: test_app crash_app forkserver simulate
	@echo "-----------------------------------------------------------------------"
	@echo "test without simulating..."
	LD_PRELOAD=./forkserver.so ./test
	([ $$? -eq 0 ] && echo "success!") || echo "failure!"
	@echo "-----------------------------------------------------------------------"
	@echo "test normal with simulating..."
	./simulate ./test
	([ $$? -eq 0 ] && echo "success!") || echo "failure!"
	@echo "-----------------------------------------------------------------------"
	@echo "test normal with simulating..."
	./simulate ./crash
	([ $$? -eq 0 ] && echo "success!") || echo "failure!"

clean:
	make -f makefile.pin clean mini pin_run
	rm -f *.so test crash simulate
