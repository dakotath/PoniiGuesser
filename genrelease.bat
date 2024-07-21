@echo off
setlocal
cls

REM Define your application name
set "appName=PONiiGuesser"

REM Clean the app.
make clean

REM Compile the app.
make

REM Generate docs
doxygen

REM Create the releases folder if it doesn't exist
if not exist "releases" mkdir "releases"

REM Create tmpFolder/apps/appName directory structure
mkdir "tmpFolder\apps\%appName%"

REM Copy carhorn.dol to tmpFolder/apps/appName/boot.dol
copy "%appName%.dol" "tmpFolder\apps\%appName%\boot.dol"

REM Copy contents of HBApp/* to tmpFolder/apps/appName/
xcopy /s /e "HBApp\*" "tmpFolder\apps\%appName%\"

REM Delete old release.
del "releases\%appName%.zip"

REM Zip tmpFolder/apps/appName to releases\appName.zip
powershell Compress-Archive -Path "tmpFolder\*" -DestinationPath "releases\%appName%.zip" -CompressionLevel Optimal

REM Delete tmpFolder
rmdir /s /q "tmpFolder"

echo Done.
pause