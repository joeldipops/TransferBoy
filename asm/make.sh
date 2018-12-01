rgbasm -o opTest.o opTest.asm
rgblink -o opTest.gb -n opTest.sym opTest.o
rgbfix -v -p 0 opTest.gb
mv opTest.gb ../filesystem/

rgbasm -o routineTest.o routineTest.asm
rgblink -o routineTest.gb -n routineTest.sym routineTest.o
rgbfix -v -p 0 routineTest.gb
mv routineTest.gb ../filesystem/
