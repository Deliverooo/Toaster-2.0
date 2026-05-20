taskkill /f /im explorer.exe
cd /d %userprofile%\AppData\Local
del /f /s /q /a Microsoft\Windows\Explorer\iconcache*
start explorer.exe