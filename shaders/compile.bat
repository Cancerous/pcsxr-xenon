@echo off

set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

echo.
echo Setting environment for using Microsoft Xbox 360 SDK tools.
echo.


echo Compile hq4x shader
fxc /Tps_3_0 ps_hq4x.hlsl /Fo ps_hq4x.psu
fxc /Tvs_3_0 vs_hq4x.hlsl /Fo vs_hq4x.vsu

echo Compile n shader
fxc /Tps_2_0 textured.hlsl /Fo textured.psu

cmd
