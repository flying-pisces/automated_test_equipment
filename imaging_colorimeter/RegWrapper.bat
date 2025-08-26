@ECHO OFF

cd %~dp0
net session >nul 2>&1
if %errorLevel% == 0 (
     regasm %windir%\Microsoft.NET\Framework\v4.0.30319\system.windows.forms.dll

     gacutil -u RadiantCommonCOM
     regasm /u "RadiantCommonCOM.dll"

     regasm /tlb "RadiantCommonCOM.dll"
     gacutil -i "RadiantCommonCOM.dll"

     gacutil -u PMEngineCOM
     regasm /u "PMEngineCOM.dll"

     regasm /tlb "PMEngineCOM.dll"
     gacutil -i "PMEngineCOM.dll"

     pause
) else (
     echo "ERROR: Current permissions inadequate. Please run as admin."
     pause 
)


