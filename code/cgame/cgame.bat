rem PATH=%PATH%;"D:\quake3(132)padtmp\bin"
set oldPATH=%PATH%
PATH=%PATH%;"..\..\..\bin"

mkdir vm
cd vm

q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_misc.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_pmove.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_slidemove.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/bg_lib.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_math.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../../game/q_shared.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_consolecmds.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_draw.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_drawtools.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_effects.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_ents.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_event.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_info.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_localents.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_main.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_marks.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_players.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_playerstate.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_predict.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_scoreboard.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_servercmds.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_snapshot.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_view.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_weapons.c
@if errorlevel 1 goto quit


q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../wop_advanced2d.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_lensflare.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_rautelib.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_spraylogo.c
@if errorlevel 1 goto quit
q3lcc -DQ3_VM -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui ../cg_spriteparticles.c
@if errorlevel 1 goto quit


q3asm -v -f ../cgame
:quit
cd ..

PATH=%oldPATH%

PAUSE