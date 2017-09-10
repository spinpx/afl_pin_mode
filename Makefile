TARGET := $(shell uname -p)
ifeq ($(TARGET),x86_64)
	PIN_OBJ_DIR="obj-intel64"
else
	PIN_OBJ_DIR="obj-ia32"
endif
TOOL_NAME=tool.so
CUR_DIR = $(shell pwd)

PIN_TOOL_PREFIX=${PIN_ROOT}/pin -t ${CUR_DIR}/${PIN_OBJ_DIR}/${TOOL_NAME} --

all: check test_app crash_app forkserver simulate pin_tool

check:
ifndef PIN_ROOT
	@echo "PIN_ROOT not defined!"
	@exit 1
endif

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
	make -f makefile.pin

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
