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
  ui_main.o \
  ui_addbots.o \
  ui_atoms.o \
  ui_callvote.o \
  ui_cdkey.o \
  ui_cinematics.o \
  ui_confirm.o \
  ui_connect.o \
  ui_controls2.o \
  ui_credits.o \
  ui_demo2.o \
  ui_display.o \
  ui_exit.o \
  ui_gameinfo.o \
  ui_ingame.o \
  ui_loadconfig.o \
  ui_menu.o \
  ui_mfield.o \
  ui_mods.o \
  ui_music.o \
  ui_network.o \
  ui_options.o \
  ui_playermodel.o \
  ui_players.o \
  ui_playersettings.o \
  ui_preferences.o \
  ui_qmenu.o \
  ui_removebots.o \
  ui_saveconfig.o \
  ui_serverinfo.o \
  ui_servers2.o \
  ui_setup.o \
  ui_sound.o \
  ui_sparena.o \
  ui_specifyserver.o \
  ui_splevel.o \
  ui_sppostgame.o \
  ui_spreset.o \
  ui_spskill.o \
  ui_startserver2.o \
  ui_team.o \
  ui_teamorders.o \
  ui_video.o \

OBJDEP = \
  ../game/q_shared.o \
  ../game/q_math.o \
  ../game/bg_misc.o \
  
SOOBJ = \
  ui_syscalls.o \

QVMOBJ = \
  ../game/bg_lib.o \

DO_VMCC = $(Q3LCC) $(VMCFLAGS) -o $@ -c $<
DO_SOCC = $(CC) $(SOCFLAGS) -o $@ -c $<

build_qvm: DO_CC=$(DO_VMCC)
build_so: DO_CC=$(DO_SOCC)

cg_consolecmds.o : cg_consolecmds.c; $(DO_CC)

ui_addbots.o : ui_addbots.c; $(DO_CC)
ui_atoms.o : ui_atoms.c; $(DO_CC)
ui_callvote.o : ui_callvote.c; $(DO_CC)
ui_cdkey.o : ui_cdkey.c; $(DO_CC)
ui_cinematics.o : ui_cinematics.c; $(DO_CC)
ui_confirm.o : ui_confirm.c; $(DO_CC)
ui_connect.o : ui_connect.c; $(DO_CC)
ui_controls2.o : ui_controls2.c; $(DO_CC)
ui_credits.o : ui_credits.c; $(DO_CC)
ui_demo2.o : ui_demo2.c; $(DO_CC)
ui_display.o : ui_display.c; $(DO_CC)
ui_exit.o : ui_exit.c; $(DO_CC)
ui_gameinfo.o : ui_gameinfo.c; $(DO_CC)
ui_ingame.o : ui_ingame.c; $(DO_CC)
ui_loadconfig.o : ui_loadconfig.c; $(DO_CC)
ui_main.o : ui_main.c; $(DO_CC)
ui_menu.o : ui_menu.c; $(DO_CC)
ui_mfield.o : ui_mfield.c; $(DO_CC)
ui_mods.o : ui_mods.c; $(DO_CC)
ui_music.o : ui_music.c; $(DO_CC)
ui_network.o : ui_network.c; $(DO_CC)
ui_options.o : ui_options.c; $(DO_CC)
ui_playermodel.o : ui_playermodel.c; $(DO_CC)
ui_players.o : ui_players.c; $(DO_CC)
ui_playersettings.o : ui_playersettings.c; $(DO_CC)
ui_preferences.o : ui_preferences.c; $(DO_CC)
ui_qmenu.o : ui_qmenu.c; $(DO_CC)
ui_removebots.o : ui_removebots.c; $(DO_CC)
ui_saveconfig.o : ui_saveconfig.c; $(DO_CC)
ui_serverinfo.o : ui_serverinfo.c; $(DO_CC)
ui_servers2.o : ui_servers2.c; $(DO_CC)
ui_setup.o : ui_setup.c; $(DO_CC)
ui_sound.o : ui_sound.c; $(DO_CC)
ui_sparena.o : ui_sparena.c; $(DO_CC)
ui_specifyserver.o : ui_specifyserver.c; $(DO_CC)
ui_splevel.o : ui_splevel.c; $(DO_CC)
ui_sppostgame.o : ui_sppostgame.c; $(DO_CC)
ui_spreset.o : ui_spreset.c; $(DO_CC)
ui_spskill.o : ui_spskill.c; $(DO_CC)
ui_startserver2.o : ui_startserver2.c; $(DO_CC)
ui_team.o : ui_team.c; $(DO_CC)
ui_teamorders.o : ui_teamorders.c; $(DO_CC)
ui_video.o : ui_video.c; $(DO_CC)

ui_syscalls.o : ui_syscalls.c; $(DO_CC)


build_qvm: $(OBJ) $(QVMOBJ)
	$(Q3ASM) -o ui.qvm $(OBJ) $(OBJDEP) $(QVMOBJ) ui_syscalls.asm
	cp *.qvm ../../wop/vm/

build_so: $(OBJ) $(SOOBJ)
	$(CC) -shared -Wl,--export-dynamic,-soname,ui$(ARCH).so -o ui$(ARCH).so $(OBJ) $(OBJDEP) $(SOOBJ)
	cp *.so ../../wop/

clean:
	rm -f *.o *.qvm *.so
