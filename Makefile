TITLE=basque

# \
!ifndef 0 # \
# nmake: \
CP=copy # \
MV=move # \
RM=del # \
CC=clang # \
SOURCE=source\$(TITLE).c # \
LIBS=-I C:\INCLUDE\ -L C:\INCLUDE\SDL2\ -l Shell32 -l C:\INCLUDE\SDL2\SDL2.lib -l C:\INCLUDE\SDL2\SDL2main.lib -l C:\INCLUDE\SDL2\SDL2_image.lib -l C:\INCLUDE\SDL2\SDL2_mixer.lib -l C:\INCLUDE\SDL2\SDL2_ttf.lib -Xlinker /SUBSYSTEM:WINDOWS # \
TARGET=-o $(TITLE).exe && mt.exe -nologo -manifest windows\$(TITLE).manifest -outputresource:$(TITLE).exe # \
!else
# make:
CP=cp -f
MV=mv -f
RM=rm -f
SDL2_FLAGS=`$$(which sdl2-config) --cflags --libs`
SOURCE=source/$(TITLE).c
LIBS=$(SDL2_FLAGS) -l SDL2_image -l SDL2_mixer -l SDL2_ttf
TARGET=-o $(TITLE)
# \
!endif

FLAGS=-Wall -Wextra -std=c99
COMPILE=$(CC) $(FLAGS) $(SOURCE) $(LIBS)
RELEASE=$(COMPILE) $(TARGET)
DEBUG=$(COMPILE) -g $(TARGET)

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
force:
	$(RELEASE)
debug: source/*.c source/*.h
	$(DEBUG)

linux:
	cp $(TITLE) linux/
	cp -r assets linux/
	zip -r $(TITLE).linux.zip linux/*
mac:
	mkdir -p mac/$(TITLE).app/Contents/Resources/
	cp $(TITLE) mac/$(TITLE).app/Contents/Resources/
	cp -r assets mac/$(TITLE).app/Contents/Resources/
	zip -r $(TITLE).mac.zip mac/$(TITLE).app
windows:
	copy $(TITLE).exe windows\ &
	robocopy assets\ windows\assets\ /e &
	powershell Compress-Archive -Force windows\* $(TITLE).windows.zip
