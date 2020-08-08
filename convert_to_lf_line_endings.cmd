REM On Unix you would do this: find ./ -type f -exec dos2unix {} \;
REM After installing dos2unix.exe in Windows, you can create a
REM small bat script with the below in it to recursively
REM change the line endings. Be careful with
REM hidden directories (e.g. .git).

for /f "tokens=* delims=" %%a in ('dir "." /s /b') do (
    dos2unix %%a
)
