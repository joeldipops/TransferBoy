# make gb roms
#cd ./asm
#./make.sh
#cd ../

./mksprite.sh
cp ./assets/*.gb ./filesystem/

# make transfer boy rom
make clean
make all

# rumble pak test
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,pak=rumble ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# controller pak test
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,mempak=/home/joeldipops/Projects/tools/cen64/mempak.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# mbc1 + ram Test ROM
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/mbc1+ram.gb,tpak_save=/home/joeldipops/Projects/tools/cen64/mbc1+ram.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# mbc5 Test ROM
~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/mbc5.gbc,tpak_save=/home/joeldipops/Projects/tools/cen64/mbc5.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# mbc3 + ram Test ROM
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/mbc3.gb,tpak_save=/home/joeldipops/Projects/tools/cen64/mbc3.sav ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64

# 2-Player Test ROM
#~/Projects/tools/cen64/cen64 -multithread -controller num=1,tpak_rom=/home/joeldipops/Projects/tools/cen64/2player.gb ~/Projects/tools/cen64/pifdata.bin  ~/Projects/TransferBoy/transferboy64.z64
