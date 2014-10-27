@Echo off
:Start
chatserver.exe
ping -n 2 localhost >NUL
echo.
echo.
echo Server crashed/exited, restarting!
echo.
goto Start