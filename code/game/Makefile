default: qvm
qvm: echotest build_qvm
so: build_so

# bins for building VMs (you need all q3lcc, q3rcc, q3cpp and q3asm in the same dir.)
# You can get them by compiling the wop codebase and copying the resulting binaries
# to the required location.
Q3LCC = ../../bin/q3lcc
Q3ASM = ../../bin/q3asm

# compiler to use for building shared objects
CC = gcc

# cflags for the VM compiler
VMCFLAGS = $(CFLAGS) -DQ3_VM
# cflags for the compiler
SOCFLAGS = $(CFLAGS) -g3 -ggdb3 -fPIC

ARCH=$(shell uname -m | sed -e s/i.86/i386/)

# version info
# this has been copied from ioQuake3's Makefile
VERSION=0.00a

USE_SVN=
ifeq ($(wildcard .svn),.svn)
  SVN_REV=$(shell LANG=C svnversion .)
  ifneq ($(SVN_REV),)
    VERSION:=$(VERSION)-r$(SVN_REV)
    USE_SVN=1
  endif
else
ifeq ($(wildcard .git/svn/.metadata),.git/svn/.metadata)
  SVN_REV=$(shell LANG=C git svn info | awk '$$1 == "Revision:" {print $$2; exit 0}')
  ifneq ($(SVN_REV),)
    VERSION:=$(VERSION)-r$(SVN_REV)
  endif
endif
endif

CFLAGS += -DBERYLLIUM_VERSION=\"$(VERSION)\"

OBJ = \
  g_main.o \
  ai_chat.o \
  ai_cmd.o \
  ai_dmnet.o \
  ai_dmq3.o \
  ai_main.o \
  ai_team.o \
  ai_vcmd.o \
  bg_misc.o \
  bg_pmove.o \
  bg_slidemove.o \
  g_active.o \
  g_arenas.o \
  g_bot.o \
  g_client.o \
  g_cmds.o \
  g_combat.o \
  g_items.o \
  g_mem.o \
  g_misc.o \
  g_missile.o \
  g_mover.o \
  g_session.o \
  g_spawn.o \
  g_stations.o \
  g_svcmds.o \
  g_target.o \
  g_team.o \
  g_trigger.o \
  g_utils.o \
  g_weapon.o \
  q_math.o \
  q_shared.o \
  g_beryllium.o \
  be_cmds.o \
  be_svcmds.o \
  be_util.o \

SOOBJ = \
  g_syscalls.o \

QVMOBJ = \
  bg_lib.o \

DO_VMCC = $(Q3LCC) $(VMCFLAGS) -o $@ -c $<
DO_SOCC = $(CC) $(SOCFLAGS) -o $@ -c $<

build_qvm: DO_CC=$(DO_VMCC)
build_so: DO_CC=$(DO_SOCC)

ai_chat.o : ai_chat.c; $(DO_CC)
ai_cmd.o : ai_cmd.c; $(DO_CC)
ai_dmnet.o : ai_dmnet.c; $(DO_CC)
ai_dmq3.o : ai_dmq3.c; $(DO_CC)
ai_main.o : ai_main.c; $(DO_CC)
ai_team.o : ai_team.c; $(DO_CC)
ai_vcmd.o : ai_vcmd.c; $(DO_CC)
bg_misc.o : bg_misc.c; $(DO_CC)
bg_pmove.o : bg_pmove.c; $(DO_CC)
bg_slidemove.o : bg_slidemove.c; $(DO_CC)
g_active.o : g_active.c; $(DO_CC)
g_arenas.o : g_arenas.c; $(DO_CC)
g_bot.o : g_bot.c; $(DO_CC)
g_client.o : g_client.c; $(DO_CC)
g_cmds.o : g_cmds.c; $(DO_CC)
g_combat.o : g_combat.c; $(DO_CC)
g_items.o : g_items.c; $(DO_CC)
g_main.o : g_main.c; $(DO_CC)
g_mem.o : g_mem.c; $(DO_CC)
g_misc.o : g_misc.c; $(DO_CC)
g_missile.o : g_missile.c; $(DO_CC)
g_mover.o : g_mover.c; $(DO_CC)
g_session.o : g_session.c; $(DO_CC)
g_spawn.o : g_spawn.c; $(DO_CC)
g_stations.o : g_stations.c; $(DO_CC)
g_svcmds.o : g_svcmds.c; $(DO_CC)
g_target.o : g_target.c; $(DO_CC)
g_team.o : g_team.c; $(DO_CC)
g_trigger.o : g_trigger.c; $(DO_CC)
g_utils.o : g_utils.c; $(DO_CC)
g_weapon.o : g_weapon.c; $(DO_CC)
q_math.o : q_math.c; $(DO_CC)
q_shared.o : q_shared.c; $(DO_CC)
g_beryllium.o : g_beryllium.c; $(DO_CC)
be_cmds.o : be_cmds.c; $(DO_CC)
be_svcmds.o : be_svcmds.c; $(DO_CC)
be_util.o : be_util.c; $(DO_CC)
  
g_syscalls.o : g_syscalls.c; $(DO_CC)

bg_lib.o : bg_lib.c; $(DO_CC)

echotest:
	echo $(QVMOBJ)

build_qvm: $(OBJ) $(QVMOBJ)
	$(Q3ASM) -o qagame.qvm $(OBJ) $(QVMOBJ) g_syscalls.asm
	cp *.qvm ../../wop/vm/

build_so: $(OBJ) $(SOOBJ)
	$(CC) -shared -Wl,--export-dynamic,-soname,qagame$(ARCH).so -o qagame$(ARCH).so $(OBJ) $(SOOBJ)
	cp *.so ../../wop/
	
clean:
	rm -f *.o *.qvm *.so
