# make gb roms
#cd ./asm
#./make.sh
#cd ../

./mksprite.sh
cp ./assets/*.gb ./filesystem/

# make transfer boy rom
make clean
make all

# Op codes test ROM
#cen64 -multithread -controller num=1,tpak_rom=/home/joel/TransferBoy/asm/opTest.gb -eep4k ~/TransferBoy/log.eeprom ~/n64devutils/cen64/data/pifdata.bin  ~/TransferBoy/transferboy64.z64

# utils routines test rom
#cen64 -multithread -controller num=1,tpak_rom=~/TransferBoy/filesystem/routineTest.gb -eep4k ~/TransferBoy/log.eeprom ~/n64devutils/cen64/data/pifdata.bin  ~/TransferBoy/transferboy64.z64

# mbc1 + ram Test ROM
~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/mbc1+ram.gb,tpak_save=/home/joeldipops/Projects/tools/cen64/mbc1+ram.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# mbc3 + ram Test ROM
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/mbc3.gb,tpak_save=/home/joeldipops/Projects/tools/cen64/mbc3.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# 2-Player Test ROM
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/2player.gb ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64
