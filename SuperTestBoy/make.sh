rgbasm -o superTestBoy.o superTestBoy.asm
rgblink -o superTestBoy.gb -n superTestBoy.sym superTestBoy.o
rgbfix -v -p 0 superTestBoy.gb
wine ~/Projects/tools/bgb/bgb64.exe
