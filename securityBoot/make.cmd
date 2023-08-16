@echo off
setlocal ENABLEEXTENSIONS

set binary=signtest
set image=device_app.bin
set target=sig_device_app.bin

if "%1" == "key" (
%binary% k
) else (
%binary% %image%
type %image%.sig > %target%
type %image% >> %target%
del %image%.sig
)

endlocal
pause