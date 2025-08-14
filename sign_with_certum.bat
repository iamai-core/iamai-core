@echo off
echo Signing with Certum certificate...
:: https://www.certum.eu/en/code-signing-certificates/

:: Set your certificate file and executable paths
set CERT_FILE=certum_certificate.p12
set EXE_FILE=build\bin\Release\chat-demo.exe

:: Check if certificate file exists
if not exist %CERT_FILE% (
    echo Error: Certificate file %CERT_FILE% not found
    echo Place your Certum .p12 file in the project root
    pause
    exit /b 1
)

:: Check if executable exists
if not exist %EXE_FILE% (
    echo Error: Executable %EXE_FILE% not found
    echo Build the project first
    pause
    exit /b 1
)

:: Prompt for password
set /p CERT_PASSWORD=Enter certificate password:

:: Find signtool (try common locations)
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"
if not exist %SIGNTOOL% set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe"
if not exist %SIGNTOOL% set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\signtool.exe"

if not exist %SIGNTOOL% (
    echo Error: signtool.exe not found
    echo Please install Windows SDK or use Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

:: Sign the executable
%SIGNTOOL% sign /f %CERT_FILE% /p %CERT_PASSWORD% /t http://time.certum.pl /fd SHA256 /v %EXE_FILE%

if %ERRORLEVEL% EQU 0 (
    echo Successfully signed %EXE_FILE%
) else (
    echo Error signing executable
)

%SIGNTOOL% verify /pa /v %EXE_FILE%

pause
