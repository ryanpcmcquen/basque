TITLE=basque

# This is a cross platform Makefile, it has been tested on Linux,
# macOS, and Windows (using make and nmake).
#
# IMPORTANT:
# - GNU/BSD make must NEVER parse Windows-only targets/prereqs (like source\*),
#   and nmake must NEVER be forced to interpret POSIX shell fragments (uname, case, backticks).
# - The reliable “one file” trick is:
#     * put the nmake/make selection in an nmake conditional
#     * hide that conditional from GNU/BSD make using the "# \" comment-continuation trick
#     * define ALL targets exactly once, and route platform differences through variables
#
# The section under '!IFDEF MAKEDIR' is for Windows (nmake), and the
# section under '!ELSE' is for everything else.

FLAGS=-Wall -Wextra -std=c99

# Shared prereq glob: keep it POSIX-style so GNU/BSD make never sees source\*
# (nmake is typically fine with forward slashes here).
DEPS=source/*

# \
!IFDEF MAKEDIR # \
# -------------------- nmake (Windows) configuration -------------------- # \
# nmake uses cmd.exe semantics in recipes. Keep POSIX shell out of here. # \
CP=copy # \
MV=move # \
RM=del /q # \
#CC="%ProgramFiles%\CheckedC-LLVM\bin\clang" # \
CC=clang # \
EMCC=%UserProfile%\code\emsdk\upstream\emscripten\emcc # \
# Windows paths/inputs for the compiler stage: # \
WIN_SOURCE=source\$(TITLE).c # \
WIN_LIBS=-I C:\INCLUDE\ -L C:\INCLUDE\SDL2\ -Xlinker windows\$(TITLE).res -l Shell32 -l C:\INCLUDE\SDL2\SDL2.lib -l C:\INCLUDE\SDL2\SDL2main.lib -l C:\INCLUDE\SDL2\SDL2_image.lib -l C:\INCLUDE\SDL2\SDL2_mixer.lib -l C:\INCLUDE\SDL2\SDL2_ttf.lib -Xlinker /SUBSYSTEM:WINDOWS # \
WIN_TARGET=-o $(TITLE).exe && mt.exe -nologo -manifest windows\$(TITLE).manifest -outputresource:$(TITLE).exe # \
# Build commands (single-line; safe to call from shared targets): # \
RELEASE_CMD=rc.exe /nologo windows\$(TITLE).rc && $(CC) $(FLAGS) $(WIN_SOURCE) $(WIN_LIBS) $(WIN_TARGET) # \
DEBUG_CMD=rc.exe /nologo windows\$(TITLE).rc && $(CC) $(FLAGS) -g $(WIN_SOURCE) $(WIN_LIBS) $(WIN_TARGET) # \
MEMDEBUG_CMD=rc.exe /nologo windows\$(TITLE).rc && $(CC) $(FLAGS) -g -fsanitize=address $(WIN_SOURCE) $(WIN_LIBS) $(WIN_TARGET) # \
# Packaging: Windows does the real thing; other platforms print guidance. # \
PACKAGE_WINDOWS_CMD=copy $(TITLE).exe windows\ & robocopy assets\ windows\assets\ /e & robocopy C:\INCLUDE\SDL2\ windows\ *.dll & powershell Compress-Archive -Force windows\* $(TITLE).windows.zip # \
PACKAGE_LINUX_CMD=echo "Use make on Linux to build/package." # \
PACKAGE_MAC_CMD=echo "Use make on macOS to build/package." # \
# WASM on Windows: keep it a no-op by default. # \
WA_CMD=echo "WASM target is configured for make/POSIX shells." # \
WASM_CMD=echo "WASM target is configured for make/POSIX shells." # \
WASMDEBUG_CMD=echo "WASM target is configured for make/POSIX shells." # \
!ELSE
# -------------------- make (Linux/macOS/BSD) configuration --------------------
CP=cp -f
MV=mv -f
RM=rm -f
CC=clang
EMCC=emcc

# Calling which here seems wrong, but somehow, in certain environments,
# it breaks without the full path ... even though the binary is in PATH.
SDL2_CONFIG=sdl2-config

# Allow overrides from the command line, e.g.:
#   make CC=gcc CPPFLAGS=... EXTRA_LIBS=...
CPPFLAGS=
EXTRA_LIBS=

# Note: On macOS, SDL2 add-on headers/libs installed via Homebrew are not
# covered by sdl2-config. We inject Homebrew include/lib paths at build time
# (in the recipe), to keep this Makefile compatible with basic make variants
# (no GNU make conditionals/functions).
POSIX_SOURCE=source/$(TITLE).c
POSIX_LINKFLAGS='-Wl,-rpath,$$ORIGIN' `$$(which $(SDL2_CONFIG)) --cflags --libs --static-libs` -l SDL2_image -l SDL2_mixer -l SDL2_ttf

# Brew injection snippet (evaluated at recipe runtime).
BREW_SNIPPET=brew_inc=""; brew_lib=""; case "$$(uname -s)" in Darwin) if command -v brew >/dev/null 2>&1; then p="$$(brew --prefix)"; brew_inc="-I$${p}/include"; brew_lib="-L$${p}/lib"; fi ;; esac;

# Build commands (single-line; safe to call from shared targets).
RELEASE_CMD=$(BREW_SNIPPET) $(CC) $(FLAGS) $(CPPFLAGS) $$brew_inc $(POSIX_SOURCE) $(POSIX_LINKFLAGS) $(EXTRA_LIBS) $$brew_lib -o $(TITLE)
DEBUG_CMD=$(BREW_SNIPPET) $(CC) $(FLAGS) -g $(CPPFLAGS) $$brew_inc $(POSIX_SOURCE) $(POSIX_LINKFLAGS) $(EXTRA_LIBS) $$brew_lib -o $(TITLE)
MEMDEBUG_CMD=$(BREW_SNIPPET) $(CC) $(FLAGS) -g -fsanitize=address $(CPPFLAGS) $$brew_inc $(POSIX_SOURCE) $(POSIX_LINKFLAGS) $(EXTRA_LIBS) $$brew_lib -o $(TITLE)

# Packaging: Linux/mac do the real thing; Windows prints guidance.
PACKAGE_WINDOWS_CMD=echo "Use nmake on Windows to build/package."
PACKAGE_LINUX_CMD=cp $(TITLE) linux/ && cp -r assets linux/ && find /usr/lib -type f -iname "*sdl2*.so.*" -exec cp {} linux/ \; && for FILE in $$(find linux/ -type f -iname "*.so.0.*"); do ln -sfv $$(basename $${FILE}) $$(echo $${FILE} | sed 's/.so.0.*/.so.0/'); done && zip -r $(TITLE).linux.zip linux/*
PACKAGE_MAC_CMD=mkdir -p mac/$(TITLE).app/Contents/Resources/ && cp $(TITLE) mac/$(TITLE).app/Contents/Resources/ && cp -r assets mac/$(TITLE).app/Contents/Resources/ && dest="mac/$(TITLE).app/Contents/Resources"; for cellar in /opt/homebrew/Cellar /usr/local/Cellar; do [ -d "$$cellar" ] || continue; find "$$cellar" -type f -iname '*sdl2*.dylib' -exec cp -f {} "$$dest/" + 2>/dev/null || true; done && zip -r $(TITLE).mac.zip mac/$(TITLE).app

WASM_TOTAL_MEMORY=512MB
WASM_STACK_MEMORY=256MB
WASM_DEBUG_TOTAL_MEMORY=1024MB
WASM_DEBUG_STACK_MEMORY=512MB

# This is just a nice shortcut for wasm without release optimizations,
# that is useful when prototyping or testing.
WA_CMD=$(EMCC) --shell-file wasm/$(TITLE)_shell.html -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
WASM_CMD=$(EMCC) --shell-file wasm/$(TITLE)_shell.html -O3 --closure 1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 -s EXPORTED_RUNTIME_METHODS=allocate --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html && zip -r $(TITLE).wasm.zip wasm/*
WASMDEBUG_CMD=$(EMCC) --shell-file wasm/$(TITLE)_shell.html -g -fsanitize=address -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_DEBUG_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_DEBUG_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html

# \
!ENDIF

# -------------------- Shared targets (defined once) --------------------
# These targets must be parseable by both make and nmake.
# Platform differences are routed through the *_CMD variables above.

$(TITLE): $(DEPS)
	$(RELEASE_CMD)

# Windows will automatically overwrite the binary when using nmake,
# but we provide clean/force for people who like that kind of thing.
clean:
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe

force: $(DEPS)
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe
	$(RELEASE_CMD)

debug: $(DEPS)
	$(DEBUG_CMD)

memdebug: $(DEPS)
	$(MEMDEBUG_CMD)

linux: $(DEPS)
	$(PACKAGE_LINUX_CMD)

mac: $(DEPS)
	$(PACKAGE_MAC_CMD)

windows: $(DEPS)
	$(PACKAGE_WINDOWS_CMD)

wa: $(DEPS)
	$(WA_CMD)

wasm: $(DEPS)
	$(WASM_CMD)

wasmdebug: $(DEPS)
	$(WASMDEBUG_CMD)
