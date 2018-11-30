C:/rgb/rgbasm.exe -o ./opTest.o opTest.asm 
C:/rgb/rgblink.exe -o ./opTest.gb -n ./opTest.sym ./opTest.o
C:/rgb/rgbfix.exe -v -p 0 ./opTest.gb

copy opTest.gb ..\filesystem\
del opTest.o opTest.gb opTest.sym

C:/rgb/rgbasm.exe -o ./routineTest.o routineTest.asm 
C:/rgb/rgblink.exe -o ./routineTest.gb -n ./routineTest.sym ./routineTest.o
C:/rgb/rgbfix.exe -v -p 0 ./routineTest.gb

copy routineTest.gb ..\filesystem\
del routineTest.o routineTest.gb routineTest.sym
