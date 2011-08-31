@echo off
rshadercompiler vs_psx.hlsl vs_psx.bin /vs

rshadercompiler ps_psx.hlsl ps_psx.bin /ps

del ..\build\shaders.o

rem pause