@echo off

set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

echo.
echo Setting environment for using Microsoft Xbox 360 SDK tools.
echo.


echo Compile hq4x shader
fxc /Fh hq4x.vs.h /Tvs_3_0 HQ4x.fx /EVS
fxc /Fh hq4x.ps.h /Tps_3_0 HQ4x.fx /EPS

echo Compile Super2xSAI shader
fxc /Fh s2xsai.vs.h /Tvs_3_0 Super2xSAI.fx /EVS
fxc /Fh s2xsai.ps.h /Tps_3_0 Super2xSAI.fx /EPS

echo Compile Scale2xPlus.fx shader
fxc /Fh scale2x.vs.h /Tvs_3_0 Scale2xPlus.fx /EVS
fxc /Fh scale2x.ps.h /Tps_3_0 Scale2xPlus.fx /EPS

echo Compile mame post shader
fxc /Fh post.vs.h /Tvs_3_0 post.fx /Evs_main
fxc /Fh post.ps.h /Tps_3_0 post.fx /Eps_main

echo Compile simple shader
fxc /Fh vs.h /Tvs_3_0 simple.fx /EVS
fxc /Fh ps.h /Tps_3_0 simple.fx /EPS

echo Compile n shader
fxc /Tps_2_0 textured.hlsl /Fo textured.psu

echo Compile psx shader
fxc /Fh psx.vs.h /Tvs_3_0 vs_psx.hlsl 
fxc /Fh psx.ps.h /Tps_3_0 ps_psx.hlsl

del ..\build\shaders.o
cmd
