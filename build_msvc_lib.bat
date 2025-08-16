@echo off

IF NOT EXIST "build" mkdir build

set sources="source\win32\platform.cpp" "source\win32\opengl_adapter.cpp" "source\font.cpp" "source\io.cpp" "source\opengl.cpp" "source\utf.cpp"
set objects="build\platform.obj" "build\opengl_adapter.obj" "build\font.obj" "build\io.obj" "build\opengl.obj" "build\utf.obj"

cl /D"DEVELOPER" /D"BOUNDS_CHECKING" /D"PLATFORM_OPENGL_INTEGRATION" /Isource /FC /Zi /nologo /W2 /permissive- /Fd"build/" /Fo"build/" /c %sources%
LIB /NOLOGO /OUT:build\mountain.lib %objects%

