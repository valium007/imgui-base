@REM Build for gcc/mingw
@set OUT_EXE=main
@set INCLUDES= -I ..\.. -I..\..\backends
@set SOURCES=main.cpp libimgui.a
clang++ %SOURCES% -o %OUT_EXE% %INCLUDES% -ld3d11 -ld3dcompiler -ldwmapi -lgdi32 -lpthread -mwindows