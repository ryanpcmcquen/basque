@echo off

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------

nuget install sdl2.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_image.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_mixer.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\
nuget install sdl2_ttf.nuget -NonInteractive -ExcludeVersion -OutputDirectory C:\INCLUDE\

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

:: System DLLs:
robocopy C:\INCLUDE\sdl2.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_image.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_image.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_mixer.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_mixer.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll

robocopy C:\INCLUDE\sdl2_ttf.nuget.redist\build\native\bin\x64\ C:\Windows\System32\ *.dll
robocopy C:\INCLUDE\sdl2_ttf.nuget.redist\build\native\bin\x64\ C:\Windows\SysWOW64\ *.dll
