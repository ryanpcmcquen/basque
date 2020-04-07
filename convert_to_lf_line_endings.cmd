REM On Unix you would do this: find ./ -type f -exec dos2unix {} \;
REM After installing dos2unix.exe in Windows, you can create a small bat script with the below in it to
REM recursively change the line endings. Careful if you have any hidden directories (e.g. .git)

for /f "tokens=* delims=" %%a in ('dir "." /s /b') do (
    dos2unix %%a
)
