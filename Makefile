TITLE=basque

# This is a cross platform Makefile, it has been tested on Linux,
# Mac OS, and Windows (using make and nmake).
#
# The section under '!IFDEF MAKEDIR' is for Windows (nmake), and the
# section under '!ELSE' is for everything else.

FLAGS=-Wall -Wextra -std=c99

# \
!IFDEF MAKEDIR
# nmake:
CP=copy
MV=move
RM=del
#CC="%ProgramFiles%\CheckedC-LLVM\bin\clang" #
CC=clang
EMCC=%UserProfile%\code\emsdk\upstream\emscripten\emcc
SOURCE=source\$(TITLE).c
LIBS=-I C:\INCLUDE\ -L C:\INCLUDE\SDL2\ -Xlinker windows\$(TITLE).res -l Shell32 -l C:\INCLUDE\SDL2\SDL2.lib -l C:\INCLUDE\SDL2\SDL2main.lib -l C:\INCLUDE\SDL2\SDL2_image.lib -l C:\INCLUDE\SDL2\SDL2_mixer.lib -l C:\INCLUDE\SDL2\SDL2_ttf.lib -Xlinker /SUBSYSTEM:WINDOWS
TARGET=-o $(TITLE).exe && mt.exe -nologo -manifest windows\$(TITLE).manifest -outputresource:$(TITLE).exe
COMPILE=rc.exe /nologo windows\$(TITLE).rc && $(CC) $(FLAGS) $(SOURCE) $(LIBS)

$(TITLE): source\*
	$(COMPILE) $(TARGET)

# Windows will automatically overwrite
# the binary when using `nmake`, but
# we add the clean command for
# people who like that kind
# of thing.
clean:
	$(RM) $(TITLE).exe

force: source\*
	$(RM) $(TITLE).exe
	nmake /nologo $(TITLE)

debug: source\*
	$(COMPILE) -g $(TARGET)

memdebug: source\*
	$(COMPILE) -g -fsanitize=address $(TARGET)

windows: source\*
	copy $(TITLE).exe windows\ &
	robocopy assets\ windows\assets\ /e &
	robocopy C:\INCLUDE\SDL2\ windows\ *.dll &
	powershell Compress-Archive -Force windows\* $(TITLE).windows.zip

# \
!ELSE
# make:
CP=cp -f
MV=mv -f
RM=rm -f
# If this fancy syntax doesn't work with your version of `make`,
# just remove the conditional wrapper (the nmake/make split).
# It works here on Linux and Mac OS.
CC=clang
EMCC=emcc
# Calling which here seems wrong, but somehow, in
# certain enviros, it breaks without the full
# path ... even though the binary is in
# the calling path.
SDL2_CONFIG=sdl2-config
SOURCE=source/$(TITLE).c
# Allow overrides from the command line, e.g.:
#   make CC=gcc CPPFLAGS=... EXTRA_LIBS=...
CPPFLAGS=
EXTRA_LIBS=
# Note: On macOS, SDL2 add-on headers/libs installed via Homebrew are not
# covered by sdl2-config. We inject Homebrew include/lib paths at build time
# (in the recipe), to keep the Makefile compatible with basic make variants
# (no GNU make conditionals/functions).
TARGET=-o $(TITLE)
$(TITLE): source/*
	brew_inc=""; brew_lib=""; \
	case "$$(uname -s)" in \
		Darwin) \
			if command -v brew >/dev/null 2>&1; then \
				p="$$(brew --prefix)"; \
				brew_inc="-I$${p}/include"; \
				brew_lib="-L$${p}/lib"; \
			fi ;; \
	esac; \
	$(CC) $(FLAGS) $(CPPFLAGS) $$brew_inc $(SOURCE) \
		'-Wl,-rpath,$$ORIGIN' \
		`$$(which $(SDL2_CONFIG)) --cflags --libs --static-libs` \
		-l SDL2_image -l SDL2_mixer -l SDL2_ttf \
		$(EXTRA_LIBS) $$brew_lib \
		$(TARGET)
# Windows will automatically overwrite
# the binary when using `nmake`, but
# we add the clean command for
# people who like that kind
# of thing.
clean:
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe
force: source/*
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe
	$(MAKE) $(TITLE)
debug: source/*
	brew_inc=""; brew_lib=""; \
	case "$$(uname -s)" in \
		Darwin) \
			if command -v brew >/dev/null 2>&1; then \
				p="$$(brew --prefix)"; \
				brew_inc="-I$${p}/include"; \
				brew_lib="-L$${p}/lib"; \
			fi ;; \
	esac; \
	$(CC) $(FLAGS) -g $(CPPFLAGS) $$brew_inc $(SOURCE) \
		'-Wl,-rpath,$$ORIGIN' \
		`$$(which $(SDL2_CONFIG)) --cflags --libs --static-libs` \
		-l SDL2_image -l SDL2_mixer -l SDL2_ttf \
		$(EXTRA_LIBS) $$brew_lib \
		$(TARGET)
memdebug: source/*
	brew_inc=""; brew_lib=""; \
	case "$$(uname -s)" in \
		Darwin) \
			if command -v brew >/dev/null 2>&1; then \
				p="$$(brew --prefix)"; \
				brew_inc="-I$${p}/include"; \
				brew_lib="-L$${p}/lib"; \
			fi ;; \
	esac; \
	$(CC) $(FLAGS) -g -fsanitize=address $(CPPFLAGS) $$brew_inc $(SOURCE) \
		'-Wl,-rpath,$$ORIGIN' \
		`$$(which $(SDL2_CONFIG)) --cflags --libs --static-libs` \
		-l SDL2_image -l SDL2_mixer -l SDL2_ttf \
		$(EXTRA_LIBS) $$brew_lib \
		$(TARGET)
linux: source/*
	cp $(TITLE) linux/
	cp -r assets linux/
	find /usr/lib -type f -iname "*sdl2*.so.*" -exec cp {} linux/ \;
	# for FILE in $$(ldd $(TITLE) | awk '{print $$3}'); do cp $$(readlink -e $$FILE) linux/; done
	for FILE in $$(find linux/ -type f -iname "*.so.0.*"); do ln -sfv $$(basename $${FILE}) $$(echo $${FILE} | sed 's/.so.0.*/.so.0/'); done
	zip -r $(TITLE).linux.zip linux/*
mac: source/*
	mkdir -p mac/$(TITLE).app/Contents/Resources/
	cp $(TITLE) mac/$(TITLE).app/Contents/Resources/
	cp -r assets mac/$(TITLE).app/Contents/Resources/
	dest="mac/$(TITLE).app/Contents/Resources"; \
	for cellar in /opt/homebrew/Cellar /usr/local/Cellar; do \
		[ -d "$$cellar" ] || continue; \
		find "$$cellar" -type f -iname '*sdl2*.dylib' -exec cp -f {} "$$dest/" + 2>/dev/null || true; \
	done
	zip -r $(TITLE).mac.zip mac/$(TITLE).app
windows: source/*
	echo "Use nmake on Windows to build/package."
WASM_TOTAL_MEMORY=512MB
WASM_STACK_MEMORY=256MB
WASM_DEBUG_TOTAL_MEMORY=1024MB
WASM_DEBUG_STACK_MEMORY=512MB
# This is just a nice shortcut for wasm without
# release optimizations, that is useful
# when prototyping or testing.
wa: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
wasm: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -O3 --closure 1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 -s EXPORTED_RUNTIME_METHODS=allocate --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
	zip -r $(TITLE).wasm.zip wasm/*
wasmdebug: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -g -fsanitize=address -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_DEBUG_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_DEBUG_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
# \
!ENDIF
