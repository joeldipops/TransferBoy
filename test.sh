# make gb roms
cd ./asm
./make.sh
cd ../

./mksprite.sh

# make transfer boy rom
make clean
make all


# Op codes test ROM
cen64 -multithread -controller num=1,tpak_rom=/home/joel/TransferBoy/filesystem/opTest.gb -eep4k ~/TransferBoy/log.eeprom ~/n64devutils/cen64/data/pifdata.bin  ~/TransferBoy/transferboy64.z64

# utils routines test rom
#cen64 -multithread -controller num=1,tpak_rom=~/TransferBoy/filesystem/routineTest.gb -eep4k ~/TransferBoy/log.eeprom ~/n64devutils/cen64/data/pifdata.bin  ~/TransferBoy/transferboy64.z64

# 2-Player Test ROM
#cen64 -multithread -controller num=1,tpak_rom=~/TransferBoy/filesystem/2player.gb -eep4k ~/TransferBoy/log.eeprom ~/n64devutils/cen64/data/pifdata.bin  ~/TransferBoy/transferboy64.z64
