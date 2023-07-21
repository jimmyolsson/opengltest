@echo off
setlocal enabledelayedexpansion
set OUTPUT_FILES=

for /R "src\" %%F in (*.cpp) do (
    set OUTPUT_FILES=!OUTPUT_FILES! %%F
	echo Added %%F
)

emcc ^
-sUSE_GLFW=3 ^
-sFULL_ES3=1 ^
-sALLOW_MEMORY_GROWTH ^
--preload-file resources ^
-Ilib\glm\include ^
-Ilib\GLFW\include ^
-Ilib\stb_image ^
-I..\..\emsdk\upstream\emscripten\cache\sysroot\include ^
-o docs\index.html ^
--shell-file docs\shell.html ^
!OUTPUT_FILES! ^

endlocal