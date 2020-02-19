TITLE=basque

# \
!ifndef 0 # \
# nmake: \
CP=copy # \
MV=move # \
RM=del # \
FLAGS=-Wall # \
SOURCE=source\$(TITLE).c # \
LIBS=-I C:\INCLUDE -L C:\INCLUDE\SDL2 -l SDL2.lib -l SDL2main.lib -l SDL2_image.lib -l SDL2_mixer.lib -l SDL2_ttf.lib -Xlinker /SUBSYSTEM:WINDOWS # \
TARGET=-o $(TITLE).exe # \
!else
# make:
CP=cp -f
MV=mv -f
RM=rm -f
FLAGS=-Wall -Wextra -std=c99
SDL2_FLAGS=`$$(which sdl2-config) --cflags --libs`
SOURCE=source/$(TITLE).c
LIBS=$(SDL2_FLAGS) -l SDL2_image -l SDL2_mixer -l SDL2_ttf
TARGET=-o $(TITLE)
# \
!endif

RELEASE=$(CC) $(SOURCE) $(FLAGS) $(LIBS) $(TARGET)

$(TITLE): source/*.c source/*.h
	$(RELEASE)
# Windows will automatically overwrite
# the binary when using `nmake`, but
# we add the clean command for
# people who like that kind
# of thing.
clean:
	$(RM) $(TITLE)
	$(RM) $(TITLE).exe
debug: source/*.c source/*.h
	$(RELEASE) -g
