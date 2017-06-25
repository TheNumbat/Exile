@echo off

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=/GR- /EHa- /nologo /FC

SET BUILD_FILE=%1
if "%BUILD_FILE%" == "" (SET BUILD_FILE="example3.cpp")

cl %OPTS% %BUILD_FILE% /Zi /Feexample