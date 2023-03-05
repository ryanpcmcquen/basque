TITLE=basque

# This is a cross platform Makefile, it has been tested on Linux,
# Mac OS, and Windows (using make and nmake).
#
# The section under '!ifndef' is for Windows, and the
# section under '!else' is for everything else.

# \
!ifndef 0 # \
# nmake: \
CP=copy # \
MV=move # \
RM=del # \
#CC="%ProgramFiles%\CheckedC-LLVM\bin\clang" # \
CC=clang # \
EMCC=%UserProfile%\code\emsdk\upstream\emscripten\emcc # \
SOURCE=source\$(TITLE).c # \
LIBS=-I C:\INCLUDE\ -L C:\INCLUDE\SDL2\ -Xlinker windows\$(TITLE).res -l Shell32 -l C:\INCLUDE\SDL2\SDL2.lib -l C:\INCLUDE\SDL2\SDL2main.lib -l C:\INCLUDE\SDL2\SDL2_image.lib -l C:\INCLUDE\SDL2\SDL2_mixer.lib -l C:\INCLUDE\SDL2\SDL2_ttf.lib -Xlinker /SUBSYSTEM:WINDOWS # \
TARGET=-o $(TITLE).exe && mt.exe -nologo -manifest windows\$(TITLE).manifest -outputresource:$(TITLE).exe # \
COMPILE=rc.exe /nologo windows\$(TITLE).rc && $(CC) $(FLAGS) $(SOURCE) $(LIBS) # \
!else
# make:
CP=cp -f
MV=mv -f
RM=rm -f
# If this fancy syntax doesn't work with your version of `make`,
# just remove the conditional wrapper (ifeq and endif).
# It works here on Linux and Mac OS.
ifeq ($(origin CC), default)
CC=clang
endif
EMCC?=emcc
# Calling which here seems wrong, but somehow, in
# certain enviros, it breaks without the full
# path ... even though the binary is in
# the calling path.
SDL2_FLAGS=`$$(which sdl2-config) --cflags --libs`
SOURCE=source/$(TITLE).c
LIBS='-Wl,-rpath,$$ORIGIN' $(SDL2_FLAGS) -l SDL2_image -l SDL2_mixer -l SDL2_ttf
ifeq ($(origin EXTRA_LIBS), default)
EXTRA_LIBS=
endif
TARGET=-o $(TITLE)
COMPILE=$(CC) $(FLAGS) $(SOURCE) $(LIBS) $(EXTRA_LIBS)
# \
!endif

FLAGS=-Wall -Wextra -std=c99
RELEASE=$(COMPILE) $(TARGET)
DEBUG=$(COMPILE) -g $(TARGET)
MEMDEBUG=$(COMPILE) -g -fsanitize=address $(TARGET)

$(TITLE): source/*
	$(RELEASE)
# Windows will automatically overwrite
# the binary when using `nmake`, but
# we add the clean command for
# people who like that kind
# of thing.
clean:
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe
force:
	$(RELEASE)

debug: source/*
	$(DEBUG)
memdebug: source/*
	$(MEMDEBUG)

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
	find /usr/local/Cellar -type f -iname "*sdl2*.dylib" -exec cp {} mac/$(TITLE).app/Contents/Resources/ \;
	zip -r $(TITLE).mac.zip mac/$(TITLE).app
windows: source/*
	copy $(TITLE).exe windows\ &
	robocopy assets\ windows\assets\ /e &
	robocopy C:\INCLUDE\SDL2\ windows\ *.dll &
	powershell Compress-Archive -Force windows\* $(TITLE).windows.zip


WASM_TOTAL_MEMORY=512MB
WASM_STACK_MEMORY=256MB
WASM_DEBUG_TOTAL_MEMORY=1024MB
WASM_DEBUG_STACK_MEMORY=512MB

# This is just a nice shortcut for wasm without
# release optimizations, that is useful
# when prototyping or testing.
wa: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 -s EXTRA_RUNTIME_METHODS=allocate --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html

wasm: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -O3 --closure 1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
	zip -r $(TITLE).wasm.zip wasm/*

wasmdebug: source/*
	$(EMCC) --shell-file wasm/$(TITLE)_shell.html -g -fsanitize=address -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_SDL_TTF=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=$(WASM_DEBUG_TOTAL_MEMORY) -s TOTAL_STACK=$(WASM_DEBUG_STACK_MEMORY) -s WASM=2 --preload-file assets $(FLAGS) -I $${HOME}/code/emsdk/upstream/emscripten/cache/sysroot/include/ -I $${HOME}/work/$(TITLE)/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ -I /builds/ryanpcmcquen/$(TITLE)/emsdk/upstream/emscripten/cache/sysroot/include/ source/$(TITLE).c -o wasm/$(TITLE).html
