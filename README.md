# TransferBoy
Game Boy Emulator for N64 using Transfer Pak. "The SuperGameBoy64 Nintendo never made"
Core emulation logic wasn't written by me but adapted from https://github.com/koenk/gbc by koenk

Runs on https://github.com/DragonMinded/libdragon by DragonMinded.

Other than those, credit to
* saturnu and https://github.com/saturnu/libgbpak
* AntonioND for "The Cycle Accurate Game Boy Docs" - https://github.com/AntonioND/giibiiadvance/tree/master/docs
* Anyone who worked on implementing the Transfer Pak in cen64
* imanoleas for their breakdown of Super Gameboy communication https://imanoleasgames.blogspot.com/2016/12/games-aside-1-super-game-boy.html
* The maintainers of https://github.com/gbdev/awesome-gbdev and pretty much anyone who has contributed to anything linked to from that page.

Font is Polygon Party https://fonts2u.com/polygon-party.font

For anyone that stumbles across this and feels like trying it out, beware that while it does successfully read in ROMs from the T-Pak and "emulate" the gameboy, it currently "runs" at about 10 frames per second, (slightly faster if you don't bother rending half the frames) not to mention a bunch of graphical issues and just plain ugliness that I've decided not to fix until I get performance under control.

In parallel I'm writing **SuperTestBoy** a game boy ROM intended to exercise each of the Super Gameboy features to make sure they are implemented as correctly as possible in Transfer Boy
