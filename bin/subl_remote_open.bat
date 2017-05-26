
@echo off
set SCRIPTDIR=%~dp0
call python "%SCRIPTDIR:~0,-1%\subl_remote_open.py" localhost 25252 %*
