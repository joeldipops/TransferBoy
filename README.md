# TransferBoy
Gameboy Emulator for N64 using Transfer Pak.  
Not much more than a wrapper around 
https://github.com/koenk/gbc and 
https://github.com/saturnu/libgbpak (heavily modified) 
that runs on https://github.com/DragonMinded/libdragon

Font is Polygon Party https://fonts2u.com/polygon-party.font

Credit to imanoleas for their breakdown of Super Gameboy communication https://imanoleasgames.blogspot.com/2016/12/games-aside-1-super-game-boy.html

For anyone that stumbles across this and feels like trying it out, beware that while it does successfully read in ROMs from the T-Pak and "emulate" the gameboy, it currently "runs" at about six frames per second, (almost 'useable' at a whopping ten fps if you dont bother rendering half the frames) not to mention a bunch of graphical issues and just plain ugliness that I've decided not to fix until I get performance under control.
