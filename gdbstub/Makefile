
all: build/$(INSTALL_LIB)stub.a

ifneq ($(MAKECMDGOALS),clean)
  $(shell mkdir -p build)
endif

build/$(INSTALL_LIB)stub.o: stub.c debug_agent.h debug_agent_interface.h debug_common.h  gdbstub.h
	gcc -Wall -Werror -g3 -fPIC -DARCH=${ARCH} -I${FAMDIR}/BE/GDB/k1c -I${TOOLS_DIR}/trace/dsu -I$(TOOLCHAIN_DIR)/include $(CFLAGS) -c $< -o $@

build/$(INSTALL_LIB)stub.a: build/$(INSTALL_LIB)stub.o
	ar rcs $@ $<

clean:
	rm -fr build

install:
	mkdir -p ${TOOLS_PREFIX}/include
	cp debug_agent.h debug_agent_interface.h debug_common.h gdbstub.h ${TOOLS_PREFIX}/include
	mkdir -p ${TOOLS_PREFIX}/$(INSTALL_LIB)
	cp build/$(INSTALL_LIB)stub.a ${TOOLS_PREFIX}/$(INSTALL_LIB)/libstub.a

