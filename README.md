# Basque

Basque is a cross-platform top-down 2d game engine.

---

Running on Windows:

![Basque Windows demo](demos/basque_windows_demo.gif "Basque Windows demo")

Running on Mac:

![Basque Mac demo](demos/basque_mac_demo.gif "Basque Mac demo")

Running on Linux:

![Basque Linux demo](demos/basque_linux_demo.gif "Basque Linux demo")

Running in your browser (thanks to WebAssembly and Emscripten):

![Basque Wasm demo](demos/basque_wasm_demo.gif "Basque Wasm demo")

[Play Basque Wasm](https://htmlpreview.github.io/?https://cdn.jsdelivr.net/gh/ryanpcmcquen/basque@master/wasm/basque.html "Play Basque Wasm")

Now with touch controls!

---

Packages for all platforms are available under the GitHub Action here:

[![Compiling ...](https://github.com/ryanpcmcquen/basque/workflows/Compiling%20.../badge.svg)](https://github.com/ryanpcmcquen/basque/actions)

Or on Itch:

https://ryanpcmcquen.itch.io/basque

---

Too lazy to do local setup? You can give it a spin using Gitpod or Replit, the only downfall will be lack of audio.

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/ryanpcmcquen/basque)

[![Run on Replit](https://replit.com/badge/github/ryanpcmcquen/basque)](https://replit.com/github/ryanpcmcquen/basque)

---

#### Global keyboard shortcuts:

<kbd>↑</kbd>: Move player North

<kbd>→</kbd>: Move player East

<kbd>↓</kbd>: Move player South

<kbd>←</kbd>: Move player West

<kbd>q</kbd>: Quit

<kbd>e</kbd>: Toggle edit mode

#### Edit mode shortcuts:

<kbd>l</kbd>: Toggle map library

<kbd>Mouse button 1</kbd> (left click): Place tile

<kbd>Mouse button 2</kbd> (right click): Select tile

---

### Why do this?

Why not just use Godot/Unity/et cetera? Basque has a very different priority list than these engines. It is _not_ a generic engine. There isn't much here, but it is a good starting point if you are looking to roll your own engine, here is what it does:

-   Compiles cross platform (Linux/Mac/Windows/WebAssembly, see [Esoteric ports](#esoteric-ports) for other platforms).
-   Visual map editing (with an easy to understand plain text format).
-   Spritesheet animation.
-   Background music.
-   Scrolling.
-   Collision detection.
-   Rudimentary frame rate limiting.
-   Accepts command line arguments (currently only for enabling fullscreen mode).
-   Resolution detection (estimates window size to 80% of the screen or uses the full resolution for fullscreen mode, this can be switched off).
-   High DPI awareness. By default the game will not run in high DPI mode, but it will be aware of high DPI contexts. This prevents things from looking fuzzy with weird scaling settings.
-   Automated GitHub and GitLab packaging pipelines.
-   Scancode based movement (rather than event loop key states). This keeps the controls from being susceptible to operating system key repeat rates and delays.
-   Unicode (UTF-8) file path support.

Everything that is here should not be considered a final or optimal solution, but rather, a simple approach that can be morphed into a better solution. At this point the engine will become more specific to the game I am building, which is why I see this as the best time to open source it. Hopefully, it can be useful to others, either as a starting point, or as a reference of some things you can accomplish with SDL2.

---

### Map editing/layouts/attributes:

The current map system utilizes three main files:

-   [map_attributes.txt](assets/data/map_attributes.txt)
-   [map_layout.txt](assets/data/map_layout.txt)
-   [map_library.txt](assets/data/map_library.txt)

While editing the map, you may use the visual map editor (available in `DEBUG_MODE` by pressing <kbd>e</kbd>) or edit the text files directly. Basque checks the timestamps of the files, and will reload them if they change, you do not need to recompile or relaunch Basque to do this. `map_layout.txt` also has suffixed versions created, with the intention of supporting future redo/undo functionality. The current number of versions that are kept is 100. This is of course, configurable.

`map_attributes.txt` follows the following format (all items are integers):

```
TILE_INDEX:
{ X_CLIP, Y_CLIP }, { NORTH_BORDER, EAST_BORDER, SOUTH_BORDER, WEST_BORDER },
```

Comments preceded with `//` in the attributes file are allowed. The other file types do not allow comments.

`map_layout.txt` and `map_library.txt` are comma separated integers, representing the tile indices. Newlines represent new rows.

For example:

```
  0,  1,  2,  3,  4,  1,  5,  _
 27, 16, 16, 16, 16, 16, 17,  _
 27, 16, 16, 16, 16, 16, 17,  _
 27, 16, 28, 16, 16, 16, 17,  _
```

The visual map editor (available in `DEBUG_MODE` by pressing <kbd>e</kbd>) automatically writes the indices with even spacing. `_` is a shortcut for a blank tile (the index for which is configurable under `configuration.h`).

`map_library.txt` is intended to be a resource for picking tiles (available in `DEBUG_MODE -> EDIT_MODE` by pressing <kbd>l</kbd>), while editing the world map (`map_layout.txt`). It has a few special properties. It will not show the player, and it always appears starting at the top left corner of the screen. It is also _not_ editable by the map editor.

---

### Basque currently requires:

-   SDL2
-   SDL2_image
-   SDL2_mixer
-   SDL2_ttf

---

### Getting SDL2 installed:

#### Linux:

##### apt:

```
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
```

##### dnf:

```
sudo dnf install SDL2-devel SDL2_image-devel SDL2_mixer-devel SDL2_ttf-devel
```

##### eopkg:

```
sudo eopkg install sdl2-devel sdl2-image-devel sdl2-mixer-devel sdl2-ttf-devel
```

##### nix:

```
nix-env --install SDL2 SDL2_image SDL2_mixer SDL2_ttf
```

Or whatever the equivalent package is for your distro.

#### Mac:

```
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf
```

#### Windows:

Run the script here: [install_sdl_on_windows.cmd](install_sdl_on_windows.cmd)

---

Or follow one of the below procedures:

##### Cmd or Powershell:

```
nuget install sdl2.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_image.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_mixer.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_ttf.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
```

```
mkdir C:\INCLUDE\SDL2\

:: Headers:
robocopy C:\INCLUDE\sdl2.nuget\build\native\include\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_image.nuget\build\native\include\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_mixer.nuget\build\native\include\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_ttf.nuget\build\native\include\ C:\INCLUDE\SDL2\

:: Libs:
robocopy C:\INCLUDE\sdl2.nuget\build\native\lib\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_image.nuget\build\native\lib\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_mixer.nuget\build\native\lib\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_ttf.nuget\build\native\lib\x64\ C:\INCLUDE\SDL2\

:: DLLs:
robocopy C:\INCLUDE\sdl2.nuget.redist\build\native\bin\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_image.nuget.redist\build\native\bin\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_mixer.nuget.redist\build\native\bin\x64\ C:\INCLUDE\SDL2\
robocopy C:\INCLUDE\sdl2_ttf.nuget.redist\build\native\bin\x64\ C:\INCLUDE\SDL2\
```

#### Cmd or Powershell as admin:

```
:: System DLLs:
robocopy C:\INCLUDE\sdl2.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_image.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_image.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_mixer.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_mixer.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_ttf.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_ttf.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll
```

##### Old school:

1. Download the latest VC development files from: https://libsdl.org

2. Place the entire contents of `include` and `lib` under `C:\INCLUDE\SDL2`.

3. Copy all DLLs under `lib` to `C:\Windows\System32` and `C:\Windows\SysWOW64`.

4. Repeat for _SDL2_image_, _SDL2_mixer_, and _SDL2_ttf_.

5. Profit.

---

### Compiling:

#### Linux & Mac:

```
make
```

#### Windows:

```
nmake
```

---

### Packaging:

To produce a zip containing the binary and all assets, run the below commands.

This should run on any system, as it includes SDL2 and the like,
but every system is different, so nothing is guaranteed.

#### Linux:

```
make && make --always-make linux
```

#### Mac:

```
make && make --always-make mac
```

#### Windows:

```
nmake && nmake /a windows
```

#### WebAssembly (requires emsdk: https://github.com/emscripten-core/emsdk):

```
make --always-make wasm
```

---

### Running:

After [compiling](#compiling) for your platform, you can launch with `./basque` (or `basque.exe` on Windows). Passing `-f` or `--fullscreen` will launch the app in fullscreen, rather than the resolution specified in [configuration.h](source/configuration.h).

### Esoteric ports:

**BSD**:

![Basque on BSD](https://user-images.githubusercontent.com/772937/91080734-f26c7400-e5fa-11ea-91bd-a1b00f589326.jpg "Basque on BSD")

**Pocketchip**:

![Basque on a PocketCHIP](https://user-images.githubusercontent.com/772937/91081932-bf2ae480-e5fc-11ea-8f92-d393e08b007a.gif "Basque on a PocketCHIP")

**Haiku**:

![Basque on Haiku](https://user-images.githubusercontent.com/772937/93433807-26def300-f87c-11ea-8b75-166ba7297b4f.png)
![Basque on Haiku](https://user-images.githubusercontent.com/772937/93433806-26465c80-f87c-11ea-824a-6afd19c35cc3.png)

**AmigaOS 4.1**:

![Basque on AmigaOS](https://user-images.githubusercontent.com/82057491/113767469-63deda00-9727-11eb-924d-49f895713ca8.png)

Thanks to [seventhwonder-amiga](https://github.com/seventhwonder-amiga)!

**Nintendo Switch**:

![Basque on Nintendo Switch](https://user-images.githubusercontent.com/88618327/131042085-efa210ad-d69b-4e9a-b1ae-56c5ba966a05.jpg)

Thanks to [fontire](https://github.com/fontire)!

#### In progress:

-   ReactOS
-   Solaris
-   TempleOS
