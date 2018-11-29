C:/rgb/rgbasm.exe -o ./test.o test.asm 
C:/rgb/rgblink.exe -o ./test.gb -n ./test.sym ./test.o
C:/rgb/rgbfix.exe -v -p 0 ./test.gb

copy test.gb ..\filesystem\
del test.o test.gb test.sym