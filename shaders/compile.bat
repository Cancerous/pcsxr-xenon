@echo off

set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

echo.
echo Setting environment for using Microsoft Xbox 360 SDK tools.
echo.

echo Compile psx shader
fxc /Fh psx.vs.h /Tvs_3_0 vs_psx.hlsl 
fxc /Fh psx.ps.h /Tps_3_0 ps_psx.hlsl /Emain
fxc /Fh psx.ps.c.h /Tps_3_0 ps_psx.hlsl /EpsC
fxc /Fh psx.ps.f.h /Tps_3_0 ps_psx.hlsl /EpsF
fxc /Fh psx.ps.g.h /Tps_3_0 ps_psx.hlsl /EpsG

del ..\build\shaders.o
cmd
