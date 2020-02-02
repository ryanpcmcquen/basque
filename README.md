# Basque

Basque is a top down 2d game engine.

---

![Basque demo](https://github.com/ryanpcmcquen/basque/raw/master/assets/images/basque_demo.gif 'Basque demo')
![Basque demo 2](https://github.com/ryanpcmcquen/basque/raw/master/assets/images/basque_demo_2.gif 'Basque demo 2')

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

Why not just use Godot/Unity/et cetera? Basque has a very different priority list than these engines. It is _not_ a generic engine. There isn't much here, but it is a good starting point if you are looking to roll your own engine, here is what it does do:

-   Map editing (with an easy to understand plain text format).
-   Spritesheet animation.
-   Background music.
-   Collision detection.
-   Some frame rate limiting (this should be improved).

At this point the engine will start becoming more specific to the game I am building, which is why I see this as the best time to open source it. Since it could be useful to others, either as a starting point, or just as a guide of how to do some things with SDL2. Note that all of the code is not necessarily the best solution to these problems, but it is _a_ solution.

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

Or whatever the equivalent package is for your distro.

#### Mac:

```
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf
```

#### Windows:

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
