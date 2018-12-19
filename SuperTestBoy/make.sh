./rgbasm.exe -o superTestBoy.o superTestBoy.asm
./rgblink.exe -o superTestBoy.gb -n superTestBoy.sym superTestBoy.o
./rgbfix.exe -v -p 0 superTestBoy.gb