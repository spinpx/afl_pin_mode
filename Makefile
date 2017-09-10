TARGET := $(shell uname -p)
ifeq ($(TARGET),x86_64)
	PIN_OBJ_DIR=obj-intel64
else
	PIN_OBJ_DIR=obj-ia32
endif
TOOL_NAME=tool.so
CUR_DIR = $(shell pwd)

#PIN_NAME=pin-3.4-97438-gf90d1f746-gcc-linux
PIN_NAME=pin-2.14-71313-gcc.4.4.7-linux
PIN_URL="http://software.intel.com/sites/landingpage/pintool/downloads/${PIN_NAME}.tar.gz"
PIN_TAR=${CUR_DIR}/${PIN_NAME}.tar.gz

PIN_ROOT=${CUR_DIR}/${PIN_NAME}
PIN_TOOL_PREFIX=${PIN_ROOT}/pin -t ${CUR_DIR}/${PIN_OBJ_DIR}/${TOOL_NAME} --

define newline


endef

define RUN_PIN_SCRIPT_BODY
#! /bin/sh
export PIN_APP_LD_PRELOAD=${CUR_DIR}/forkserver.so
${PIN_TOOL_PREFIX} CMD_HERE
endef

RUN_PIN_SCRIPT="$(subst $(newline),\n,${RUN_PIN_SCRIPT_BODY})"
RUN_PIN_FILE="${CUR_DIR}/pin_run"

all: check test_app crash_app forkserver simulate pin_tool

check: | ${PIN_ROOT}

${PIN_ROOT}:
	@echo "install pin ...."
	@wget ${PIN_URL}
	@tar -xf ${PIN_TAR}
  # write pin run file.
	rm -rf pin_run
	@echo ${RUN_PIN_SCRIPT} > ${RUN_PIN_FILE}
	@sed -i -e 's/CMD_HERE/@#/g' ${RUN_PIN_FILE}
	@chmod 777 ${RUN_PIN_FILE}

forkserver: forkserver.cpp
	${CXX} -O3 -std=c++11 -Wall -shared -fPIC -o forkserver.so forkserver.cpp

simulate: simulate.cpp
	$(CXX) -O3 -std=c++11 -o simulate simulate.cpp

test_app: test.cpp
	$(CXX) -O3 -o test test.cpp

crash_app: crash.cpp
	$(CXX) -O3 -o crash crash.cpp

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
	make -f makefile.pin clean
	rm -f *.so test crash simulate
