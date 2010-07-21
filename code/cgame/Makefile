default: qvm
qvm: build_qvm
so: build_so

# bins for building VMs (you need all q3lcc, q3rcc, q3cpp and q3asm in the same dir.)
Q3LCC = ../../bin/q3lcc
Q3ASM = ../../bin/q3asm

# compiler to use for building shared objects
CC = gcc

# cflags for the VM compiler
VMCFLAGS = $(CFLAGS) -DQ3_VM
# cflags for the compiler
SOCFLAGS = $(CFLAGS) -g3 -ggdb3 -fPIC

ARCH=$(shell uname -m | sed -e s/i.86/i386/)

OBJ = \
  cg_main.o \
  cg_consolecmds.o \
  cg_draw.o \
  cg_drawtools.o \
  cg_effects.o \
  cg_ents.o \
  cg_event.o \
  cg_info.o \
  cg_lensflare.o \
  cg_localents.o \
  cg_marks.o \
  cg_players.o \
  cg_playerstate.o \
  cg_predict.o \
  cg_rautelib.o \
  cg_scoreboard.o \
  cg_servercmds.o \
  cg_snapshot.o \
  cg_spraylogo.o \
  cg_spriteparticles.o \
  cg_view.o \
  cg_weapons.o \
  wop_advanced2d.o \

OBJDEP = \
  ../game/q_shared.o \
  ../game/q_math.o \
  ../game/bg_misc.o \
  ../game/bg_pmove.o \
  ../game/bg_slidemove.o \
  
SOOBJ = \
  cg_syscalls.o \

QVMOBJ = \
  ../game/bg_lib.o \

DO_VMCC = $(Q3LCC) $(VMCFLAGS) -o $@ -c $<
DO_SOCC = $(CC) $(SOCFLAGS) -o $@ -c $<

build_qvm: DO_CC=$(DO_VMCC)
build_so: DO_CC=$(DO_SOCC)

cg_consolecmds.o : cg_consolecmds.c; $(DO_CC)
cg_draw.o : cg_draw.c; $(DO_CC)
cg_drawtools.o : cg_drawtools.c; $(DO_CC)
cg_effects.o : cg_effects.c; $(DO_CC)
cg_ents.o : cg_ents.c; $(DO_CC)
cg_event.o : cg_event.c; $(DO_CC)
cg_info.o : cg_info.c; $(DO_CC)
cg_lensflare.o : cg_lensflare.c; $(DO_CC)
cg_localents.o : cg_localents.c; $(DO_CC)
cg_main.o : cg_main.c; $(DO_CC)
cg_marks.o : cg_marks.c; $(DO_CC)
cg_players.o : cg_players.c; $(DO_CC)
cg_playerstate.o : cg_playerstate.c; $(DO_CC)
cg_predict.o : cg_predict.c; $(DO_CC)
cg_rautelib.o : cg_rautelib.c; $(DO_CC)
cg_scoreboard.o : cg_scoreboard.c; $(DO_CC)
cg_servercmds.o : cg_servercmds.c; $(DO_CC)
cg_snapshot.o : cg_snapshot.c; $(DO_CC)
cg_spraylogo.o : cg_spraylogo.c; $(DO_CC)
cg_spriteparticles.o : cg_spriteparticles.c; $(DO_CC)
cg_view.o : cg_view.c; $(DO_CC)
cg_weapons.o : cg_weapons.c; $(DO_CC)
wop_advanced2d.o : wop_advanced2d.c; $(DO_CC)

cg_syscalls.o : cg_syscalls.c; $(DO_CC)


build_qvm: $(OBJ) $(QVMOBJ)
	$(Q3ASM) -o cgame.qvm $(OBJ) $(OBJDEP) $(QVMOBJ) cg_syscalls.asm
	cp *.qvm ../../wop/vm/

build_so: $(OBJ) $(SOOBJ)
	$(CC) -shared -Wl,--export-dynamic,-soname,cgame$(ARCH).so -o cgame$(ARCH).so $(OBJ) $(OBJDEP) $(SOOBJ)
	cp *.so ../../wop/

clean:
	rm -f *.o *.qvm *.so
