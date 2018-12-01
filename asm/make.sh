make
cp opTest.gb ../filesystem/
rm opTest.o opTest.gb opTest.sym
rgbasm -o routineTest.o routineTest.asm
rgblink -o routineTest.gb -n routineTest.sym routineTest.o
rgbfix -v -p 0 ./routineTest.gb
cp routineTest.gb ../filesystem/
rm routineTest.o routineTest.gb routineTest.sym
