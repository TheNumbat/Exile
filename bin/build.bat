@echo off

if not exist w:\build mkdir w:\build
pushd w:\build

if "%2"=="SDL" (
	set SDL_COMP=-DPLATFORM_SDL -Iw:\deps\
	set SDL_LINK=/LIBPATH:w:\deps\SDL2\lib\ SDl2.lib SDL2main.lib
	set SDL_META=-DPLATFORM_SDL
)

set Game_CompilerFlags=%1 %SDL_COMP% -MTd -nologo -fp:fast -GR- -EHa- -Oi -W4 -Z7 -FC -Fegame.dll -LD -wd4100 -wd4201 -Iw:\build\ -D_HAS_EXCEPTIONS=0 -DCHECKS -DPROFILE
set Game_LinkerFlags=/NODEFAULTLIB:MSVCRT %SDL_LINK% /SUBSYSTEM:windows opengl32.lib -PDB:game_%random%.pdb 

set Platform_CompilerFlags=%1 %SDL_COMP% -Z7 -MTd -nologo -fp:fast -GR- -EHa- -W4 -FC -Femain.exe -wd4100 -wd4530 -wd4577 -DTEST_NET_ZERO_ALLOCS
set Platform_LinkerFlags=/NODEFAULTLIB:MSVCRT %SDL_LINK% /SUBSYSTEM:console user32.lib gdi32.lib opengl32.lib kernel32.lib

set Asset_CompilerFlags=-O2 -MTd -nologo -EHsc -Oi -W4 -Z7 -FC -Feasset.exe -wd4100 -Iw:\deps\
set Asset_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console

set Meta_CompilerFlags=-O2 -MTd -nologo -EHsc -Oi -W4 -Z7 -FC -Femeta.exe -wd4100 -Iw:\deps\
set Meta_LinkerFlags=/NODEFAULTLIB:MSVCRT /SUBSYSTEM:console /LIBPATH:w:\deps\clang-c libclang.lib

set Test_CompilerFlags=-O2 -MTd -nologo -EHsc -Oi -W4 -Z7 -FC -Iw:\build\
set Test_LinkerFlags=/SUBSYSTEM:console 

echo ASSET BUILDER 
echo.
(
	if not exist asset.exe (
		echo compiling
		cl %Asset_CompilerFlags% w:\src\tools\asset_builder.cpp /link %Asset_LinkerFlags%
	)

	if not exist w:\data\assets\engine.asset (
		echo buliding engine.asset
		asset.exe w:\data\assets\engine.txt w:\data\assets\engine.asset
	)

	if not exist w:\data\assets\game.asset (
		echo buliding game.asset
		asset.exe w:\data\assets\game.txt w:\data\assets\game.asset
	)
)
echo.
echo META 
echo.
(
	if not exist meta.exe (
		echo compiling
		cl %Meta_CompilerFlags% w:\src\tools\meta.cpp /link %Meta_LinkerFlags%
	)

	echo running
	xcopy w:\deps\clang-c\libclang.dll w:\build\ /C /Y > NUL 2> NUL
	meta.exe w:\src\compile.cpp %SDL_META%
)
echo.
echo TESTS 
echo.
(
	for %%f in (w:\src\engine\test\*.cpp) do (
	
		if not exist %%~nf.exe (
			cl %Test_CompilerFlags% -Fe%%~nf.exe %%f /link %Test_LinkerFlags%

			%%~nf.exe > %%~nf.txt
			if ERRORLEVEL 1 echo %%~nf FAILED && goto done
			echo %%~nf PASSED
		)
	)
)
echo. 
echo GAME 
echo.
(
	cl %Game_CompilerFlags% w:\src\compile.cpp /link %Game_LinkerFlags%
)
echo.
echo PLATFORM 
echo.
(
	if not exist main.exe (
		echo compiling 
		cl %Platform_CompilerFlags% w:\src\engine\platform\platform_main.cpp /link %Platform_LinkerFlags%
	)
)
echo.

:done
popd
