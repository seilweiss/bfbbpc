# BFBBPC
This is a project to decompile and port SpongeBob SquarePants: Battle for Bikini Bottom to Windows. It is based on the GameCube version of the game (the worst version), but some features from the Xbox version (the best version) are planned to be reimplemented, such as post-processing (bloom) and audio effects (reverb in levels such as Mermalair).

I am aiming to have the port completed by October 31, 2023 (the 20 year anniversary of the original game's release). I am not currently looking for contributors, but any support (especially testing) is appreciated! Join the [BFBB Decompilation](https://discord.gg/dVbGFdYU6A) or [Heavy Iron Modding](https://discord.gg/9eAE6UB) Discord servers for more information and progress updates.

This project is not based on the [BFBB Decomp Project](https://github.com/bfbbdecomp/bfbb) since all of the decompiled code so far has been written from the ground up, but much of my knowledge about this game and reverse engineering in general is from contributing to that project.

## Building
### Prerequisites
- A copy of the original game for Xbox
- [Visual Studio 2022](https://visualstudio.microsoft.com/)
- [RenderWare SDK 3.4 for D3D8](https://archive.org/details/rw34sdk) - Download the ZIP and extract the `RW34` folder into the `vendor` folder.
- [SDL 2.26.3](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.3) - Download `SDL2-devel-2.26.3-VC.zip` and extract the `SDL2-2.26.3` folder into the `vendor` folder.
- [DirectX SDK August 2007](https://archive.org/details/dxsdk_aug2007) - Download and install the SDK.

Building is simple: Open bfbbpc.sln in Visual Studio and build any of the 3 configurations: Debug, Release, and Master.
- Debug (`sbpcD.exe`) is the least optimized version with the most debug features (asserts, logging, etc.)
- Release (`sbpcR.exe`) is a more optimized version of Debug with less debug features
- Master (`sb.exe`/`sbpcM.exe`) is the most optimized version with no debug features (the version that normally appears on a retail disc)

## Converting assets
Currently you will need the Xbox version of the original game and some way to extract the files from it. You'll need to extract the files directly into the `bin` folder (`sb.ini`, `boot.HIP`, `font.HIP`, etc. should all be in the same folder as the game exes).

In order for textures to appear correctly, you'll also need to convert all the RWTX assets in the HIP/HOP files to Direct3D8 3.4.0.3 (same version as GTA Vice City). I recommend using [Industrial Park](https://github.com/igorseabra4/IndustrialPark) to export and reimport the textures and [Magic.TXD](https://www.gtagarage.com/mods/show.php?id=27862) to convert them.

It's a complicated process at the moment. There will eventually be a converter tool to do this automatically and possibly support all versions of the game. In the meantime, join the [BFBB Decompilation](https://discord.gg/dVbGFdYU6A) or [Heavy Iron Modding](https://discord.gg/9eAE6UB) Discord servers if you need help and we can walk you through it.
