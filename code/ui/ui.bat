rem PATH=%PATH%;"C:\quake3(132)padtmp\bin"
set oldPATH=%PATH%
PATH=%PATH%;"..\..\..\bin"

mkdir vm
cd vm

q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_main.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_cdkey.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_ingame.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_serverinfo.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_confirm.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_setup.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_misc.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_lib.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_math.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_shared.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_gameinfo.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_atoms.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_connect.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_controls2.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_demo2.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_mfield.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_credits.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_menu.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_options.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_display.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_sound.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_network.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_playermodel.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_players.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_playersettings.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_preferences.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_qmenu.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_servers2.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_sparena.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_specifyserver.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_splevel.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_sppostgame.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_startserver2.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_team.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_video.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_cinematics.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_spskill.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_addbots.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_removebots.c
@if errorlevel 1 goto quit
rem q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_loadconfig.c
rem @if errorlevel 1 goto quit
rem q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_saveconfig.c
rem @if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_teamorders.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_mods.c
@if errorlevel 1 goto quit

q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_exit.c
@if errorlevel 1 goto quit

q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_music.c
@if errorlevel 1 goto quit

q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../ui_callvote.c
@if errorlevel 1 goto quit

q3lcc -DQ3_VM -D_UI -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../cgame/wop_advanced2d.c
@if errorlevel 1 goto quit

q3asm -v -f ../ui
:quit
cd ..

PATH=%oldPATH%

PAUSE