@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "RELEASE_MODE=0"
if /I "%~1"=="release" set "RELEASE_MODE=1"
if not "%~1"=="" if "%RELEASE_MODE%"=="0" goto :usage

title WinterStatic yt-dlp Downloader - Native Build 0.1.39

echo.
echo ==========================================================
echo   WinterStatic yt-dlp Downloader - Native Win32 0.1.39
echo ==========================================================
echo.
echo Builds the native Win32 frontend with MSVC and prepares the portable folder.
if "%RELEASE_MODE%"=="1" echo Release mode also creates GitHub-ready portable and source ZIP assets.
echo.

where cl.exe >nul 2>nul
if not errorlevel 1 goto :compiler_ready

set "VSINSTALL="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%I"
)
if not defined VSINSTALL if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
if not defined VSINSTALL if exist "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools"
if not defined VSINSTALL if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
if not defined VSINSTALL if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
if not defined VSINSTALL if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
if not defined VSINSTALL goto :no_msvc

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 goto :env_failed

:compiler_ready
where cl.exe >nul 2>nul
if errorlevel 1 goto :no_msvc
where rc.exe >nul 2>nul
if errorlevel 1 goto :no_rc

set "BUILD_WORK=%TEMP%\WinterStatic-Downloader-build-%RANDOM%-%RANDOM%"
mkdir "%BUILD_WORK%" >nul 2>nul
if errorlevel 1 goto :build_failed

set "BUILD_RES=%BUILD_WORK%\app.res"
set "BUILD_OBJ=%BUILD_WORK%\main.obj"
set "BUILD_EXE=%BUILD_WORK%\WinterStatic-yt-dlp-Downloader.exe"

echo [1/5] Compiling resources...
rc /nologo /fo "%BUILD_RES%" resource.rc
if errorlevel 1 goto :build_failed

echo [2/5] Compiling native frontend...
cl /nologo /std:c++17 /O2 /EHsc /MT /utf-8 /DUNICODE /D_UNICODE /Fo"%BUILD_OBJ%" main.cpp "%BUILD_RES%" ^
  /link /SUBSYSTEM:WINDOWS /OUT:"%BUILD_EXE%" ^
  User32.lib Gdi32.lib Dwmapi.lib UxTheme.lib Comdlg32.lib Comctl32.lib Shell32.lib Shlwapi.lib Ole32.lib
if errorlevel 1 goto :build_failed
if not exist "%BUILD_EXE%" goto :build_failed

set "BASE_DIST=%CD%\WinterStatic-yt-dlp-Downloader-0.1.39-portable"
set "DIST=%BASE_DIST%"
if not exist "%DIST%" goto :dist_ready
rmdir /s /q "%DIST%" >nul 2>nul
if not exist "%DIST%" goto :dist_ready
set /a SUFFIX=2
:find_dist
set "DIST=%BASE_DIST%-%SUFFIX%"
if not exist "%DIST%" goto :dist_ready
set /a SUFFIX+=1
goto :find_dist

:dist_ready
mkdir "%DIST%"
if errorlevel 1 goto :package_failed
copy /y "%BUILD_EXE%" "%DIST%\WinterStatic-yt-dlp-Downloader.exe" >nul
if errorlevel 1 goto :package_failed
copy /y "README.md" "%DIST%\README.txt" >nul
if errorlevel 1 goto :package_failed
copy /y "LICENSE" "%DIST%\LICENSE" >nul
if errorlevel 1 goto :package_failed
copy /y "THIRD-PARTY-NOTICE.txt" "%DIST%\THIRD-PARTY-NOTICE.txt" >nul
if errorlevel 1 goto :package_failed

> "%DIST%\settings.ini" (
  echo [General]
  echo OutputDir=
  echo YtDlpPath=Tools\yt-dlp\yt-dlp.exe
  echo FfmpegPath=
  echo Quality=Maximum 1080p
  echo Authentication=Automatic
  echo ClosePowerShellOnSuccess=0
  echo BrowserAssistedRecovery=0
)

mkdir "%DIST%\Tools\yt-dlp" >nul 2>nul
mkdir "%DIST%\Tools\BrowserProfile" >nul 2>nul
mkdir "%DIST%\Tools\FFmpeg\bin" >nul 2>nul

rem Seed from existing local runtime tools when available. The primary yt-dlp
rem is deliberately refreshed to the latest official stable below; an existing
rem FFmpeg is retained so the generated DIST stays portable without needless work.
echo [3/5] Preparing portable Tools folder...
if exist "%CD%\Tools\" (
    robocopy "%CD%\Tools" "%DIST%\Tools" /E /NFL /NDL /NJH /NJS /NP >nul
    if errorlevel 8 goto :package_failed
) else if exist "%CD%\..\Tools\" (
    robocopy "%CD%\..\Tools" "%DIST%\Tools" /E /NFL /NDL /NJH /NJS /NP >nul
    if errorlevel 8 goto :package_failed
)

rem Never carry personal browser data or an obsolete LibreWolf tree into a new package.
if exist "%DIST%\Tools\BrowserProfile" rmdir /s /q "%DIST%\Tools\BrowserProfile" >nul 2>nul
for /d %%D in ("%DIST%\Tools\BrowserProfile_backup_*") do if exist "%%~fD" rmdir /s /q "%%~fD" >nul 2>nul
mkdir "%DIST%\Tools\BrowserProfile" >nul 2>nul
if exist "%DIST%\Tools\LibreWolf" rmdir /s /q "%DIST%\Tools\LibreWolf" >nul 2>nul
mkdir "%DIST%\Tools\yt-dlp" >nul 2>nul
mkdir "%DIST%\Tools\FFmpeg\bin" >nul 2>nul

echo [4/5] Ensuring portable yt-dlp backends...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $ProgressPreference='SilentlyContinue'; [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; $primary=Join-Path $env:DIST 'Tools\yt-dlp\yt-dlp.exe'; $auth=Join-Path $env:DIST 'Tools\yt-dlp\yt-dlp-auth.exe'; $min=[version]'2026.8.19'; $headers=@{'User-Agent'='WinterStatic-yt-dlp-Downloader-build'}; $stableUrl='https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe'; $authFallbackVersion='2026.07.04'; $authBlacklist=@('2026.08.19'); $tmp=$primary + '.download.exe'; $authTmp=$auth + '.download.exe'; if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force }; if (Test-Path -LiteralPath $authTmp) { Remove-Item -LiteralPath $authTmp -Force }; try { Invoke-WebRequest -UseBasicParsing -Headers $headers -Uri $stableUrl -OutFile $tmp; $pv=([string](& $tmp --version 2>$null | Select-Object -First 1)).Trim(); $pver=[version]$pv; if (-not $pv -or -not $pver -or $pver -lt $min -or $pv -notmatch '^\d{4}\.\d{2}\.\d{2}$') { throw ('Primary latest stable is not acceptable: ' + $pv) }; if ($authBlacklist -contains $pv) { $authUrl='https://github.com/yt-dlp/yt-dlp/releases/download/' + $authFallbackVersion + '/yt-dlp.exe'; Invoke-WebRequest -UseBasicParsing -Headers $headers -Uri $authUrl -OutFile $authTmp; $av=([string](& $authTmp --version 2>$null | Select-Object -First 1)).Trim(); if ($av -ne $authFallbackVersion) { throw ('Authenticated fallback did not report ' + $authFallbackVersion + ': ' + $av) }; Write-Host ('      Authenticated backend: stable ' + $pv + ' is blacklisted; using ' + $av + '.') -ForegroundColor Yellow } else { Copy-Item -LiteralPath $tmp -Destination $authTmp -Force; $av=([string](& $authTmp --version 2>$null | Select-Object -First 1)).Trim(); if ($av -ne $pv -or $authBlacklist -contains $av -or $av -notmatch '^\d{4}\.\d{2}\.\d{2}$') { throw ('Authenticated stable is not acceptable: ' + $av) }; Write-Host ('      Authenticated backend follows latest stable: ' + $av) }; Move-Item -LiteralPath $tmp -Destination $primary -Force; Move-Item -LiteralPath $authTmp -Destination $auth -Force; Write-Host ('      Primary yt-dlp latest stable ready: ' + $pv) } finally { if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force -ErrorAction SilentlyContinue }; if (Test-Path -LiteralPath $authTmp) { Remove-Item -LiteralPath $authTmp -Force -ErrorAction SilentlyContinue } }"
if errorlevel 1 goto :ytdlp_download_failed
if not exist "%DIST%\Tools\yt-dlp\yt-dlp.exe" goto :ytdlp_download_failed
if not exist "%DIST%\Tools\yt-dlp\yt-dlp-auth.exe" goto :ytdlp_download_failed

echo [5/5] Ensuring portable FFmpeg...
if exist "%DIST%\Tools\FFmpeg\bin\ffmpeg.exe" goto :ffmpeg_ready

echo       Downloading Gyan FFmpeg release essentials build...
set "FFMPEG_ZIP=%BUILD_WORK%\ffmpeg-release-essentials.zip"
set "FFMPEG_UNPACK=%BUILD_WORK%\ffmpeg-unpack"
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $ProgressPreference='SilentlyContinue'; [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -UseBasicParsing -Uri 'https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip' -OutFile $env:FFMPEG_ZIP; Expand-Archive -LiteralPath $env:FFMPEG_ZIP -DestinationPath $env:FFMPEG_UNPACK -Force; $bin = Get-ChildItem -LiteralPath $env:FFMPEG_UNPACK -Filter ffmpeg.exe -File -Recurse | Select-Object -First 1; if (-not $bin) { throw 'ffmpeg.exe was not found in the downloaded archive' }; $dest = Join-Path $env:DIST 'Tools\FFmpeg\bin'; Copy-Item -LiteralPath $bin.FullName -Destination (Join-Path $dest 'ffmpeg.exe') -Force; $probe = Get-ChildItem -LiteralPath $env:FFMPEG_UNPACK -Filter ffprobe.exe -File -Recurse | Select-Object -First 1; if ($probe) { Copy-Item -LiteralPath $probe.FullName -Destination (Join-Path $dest 'ffprobe.exe') -Force }; $play = Get-ChildItem -LiteralPath $env:FFMPEG_UNPACK -Filter ffplay.exe -File -Recurse | Select-Object -First 1; if ($play) { Copy-Item -LiteralPath $play.FullName -Destination (Join-Path $dest 'ffplay.exe') -Force }"
if errorlevel 1 goto :ffmpeg_download_failed
if not exist "%DIST%\Tools\FFmpeg\bin\ffmpeg.exe" goto :ffmpeg_download_failed
goto :ffmpeg_validated

:ffmpeg_ready
echo       Existing Tools\FFmpeg\bin\ffmpeg.exe retained.

:ffmpeg_validated

rem Validate the portable runtime entries before declaring success.
"%DIST%\Tools\yt-dlp\yt-dlp.exe" --version >nul 2>nul
if errorlevel 1 goto :ytdlp_download_failed
powershell -NoProfile -Command "$pv=([string](& '%DIST%\Tools\yt-dlp\yt-dlp.exe' --version 2>$null | Select-Object -First 1)).Trim(); $av=([string](& '%DIST%\Tools\yt-dlp\yt-dlp-auth.exe' --version 2>$null | Select-Object -First 1)).Trim(); $blacklist=@('2026.08.19'); if ($pv -notmatch '^\d{4}\.\d{2}\.\d{2}$') { exit 1 }; if ($blacklist -contains $pv) { if ($av -ne '2026.07.04') { exit 1 } } else { if ($av -ne $pv -or $blacklist -contains $av -or $av -notmatch '^\d{4}\.\d{2}\.\d{2}$') { exit 1 } }"
if errorlevel 1 goto :ytdlp_download_failed
"%DIST%\Tools\FFmpeg\bin\ffmpeg.exe" -version >nul 2>nul
if errorlevel 1 goto :ffmpeg_download_failed

> "%DIST%\BUILD-INFO.txt" (
  echo WinterStatic yt-dlp Downloader
  echo Version 0.1.39
  echo.
  echo Created by WinterStatic
  echo Native Win32 frontend
  echo Project: https://github.com/WinterStatic/yt-dlp-WinterStatic
  echo Frontend license: MIT
  echo.
  echo Runtime tools are kept in the Tools folder. yt-dlp is refreshed from the official latest stable during the build; FFmpeg is downloaded when missing.
  echo Public/anonymous YouTube uses the latest official stable yt-dlp backend - never nightly.
  echo Authenticated YouTube uses a separate stable-only Tools\yt-dlp\yt-dlp-auth.exe; the latest stable is used unless blacklisted, currently 2026.08.19, with 2026.07.04 as the fallback.
  echo The frontend itself has no external DLL dependency beyond Windows system libraries.
)

if not exist "%DIST%\WinterStatic-yt-dlp-Downloader.exe" goto :package_failed
if not exist "%DIST%\README.txt" goto :package_failed
if not exist "%DIST%\LICENSE" goto :package_failed
if not exist "%DIST%\THIRD-PARTY-NOTICE.txt" goto :package_failed
if not exist "%DIST%\settings.ini" goto :package_failed
if not exist "%DIST%\BUILD-INFO.txt" goto :package_failed
if not exist "%DIST%\Tools\yt-dlp\yt-dlp.exe" goto :package_failed
if not exist "%DIST%\Tools\yt-dlp\yt-dlp-auth.exe" goto :package_failed
if not exist "%DIST%\Tools\FFmpeg\bin\ffmpeg.exe" goto :package_failed

if "%RELEASE_MODE%"=="1" (
    call :make_release_assets
    if errorlevel 1 goto :release_failed
)

rmdir /s /q "%BUILD_WORK%" >nul 2>nul

echo.
echo ==========================================================
echo BUILD SUCCESSFUL
echo ==========================================================
echo.
echo Portable folder:
echo   %DIST%
if "%RELEASE_MODE%"=="1" (
    echo.
    echo GitHub release assets:
    echo   %RELEASE_PORTABLE_ZIP%
    echo   %RELEASE_SOURCE_ZIP%
)
echo.
echo Microsoft Edge is used through a dedicated Tools\BrowserProfile directory.
echo Portable primary and authenticated yt-dlp stable backends, plus FFmpeg, are present under Tools.
echo Public/anonymous YouTube uses the primary backend; authenticated YouTube switches to yt-dlp-auth.exe.
echo Run WinterStatic-yt-dlp-Downloader.exe.
echo.
pause
exit /b 0

:make_release_assets
echo.
echo [release] Creating GitHub release ZIPs...
set "RELEASE_PORTABLE_ZIP=%CD%\WinterStatic-yt-dlp-Downloader-0.1.39-portable.zip"
set "RELEASE_SOURCE_ZIP=%CD%\WinterStatic-yt-dlp-Downloader-0.1.39-source.zip"
set "RELEASE_STAGE=%BUILD_WORK%\release-assets"
set "RELEASE_PORTABLE_DIR=%RELEASE_STAGE%\WinterStatic-yt-dlp-Downloader-0.1.39-portable"
set "RELEASE_SOURCE_DIR=%RELEASE_STAGE%\WinterStatic-yt-dlp-Downloader-0.1.39-source"

if exist "%RELEASE_STAGE%" rmdir /s /q "%RELEASE_STAGE%" >nul 2>nul
mkdir "%RELEASE_PORTABLE_DIR%" >nul 2>nul
if errorlevel 1 exit /b 1
mkdir "%RELEASE_SOURCE_DIR%" >nul 2>nul
if errorlevel 1 exit /b 1

rem Stage the validated portable folder under a canonical release-folder name,
rem even if the working DIST had to use a numeric suffix because an old folder was locked.
robocopy "%DIST%" "%RELEASE_PORTABLE_DIR%" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 exit /b 1

rem The source kit is explicit on purpose: no Tools binaries, browser profile,
rem generated portable folder, downloaded media, or other local state can leak in.
for %%F in (.gitignore LICENSE PUBLIC-RELEASE-CHECKLIST.md README.md THIRD-PARTY-NOTICE.txt app-arrow-transparent.png yt-dlp_winterstatic.png app.ico app.manifest build-native.bat main.cpp resource.rc settings.ini) do (
    if not exist "%%F" (
        echo ERROR: Required source-release file is missing: %%F
        exit /b 1
    )
    copy /y "%%F" "%RELEASE_SOURCE_DIR%\%%F" >nul
    if errorlevel 1 exit /b 1
)

if exist "%RELEASE_PORTABLE_ZIP%" del /f /q "%RELEASE_PORTABLE_ZIP%" >nul 2>nul
if exist "%RELEASE_SOURCE_ZIP%" del /f /q "%RELEASE_SOURCE_ZIP%" >nul 2>nul
if exist "%RELEASE_PORTABLE_ZIP%" exit /b 1
if exist "%RELEASE_SOURCE_ZIP%" exit /b 1

powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; Compress-Archive -LiteralPath $env:RELEASE_PORTABLE_DIR -DestinationPath $env:RELEASE_PORTABLE_ZIP -CompressionLevel Optimal -Force; Compress-Archive -LiteralPath $env:RELEASE_SOURCE_DIR -DestinationPath $env:RELEASE_SOURCE_ZIP -CompressionLevel Optimal -Force"
if errorlevel 1 exit /b 1
if not exist "%RELEASE_PORTABLE_ZIP%" exit /b 1
if not exist "%RELEASE_SOURCE_ZIP%" exit /b 1

echo       Portable ZIP: %RELEASE_PORTABLE_ZIP%
echo       Source ZIP:   %RELEASE_SOURCE_ZIP%
exit /b 0

:usage
echo.
echo ERROR: Unknown build option: %~1
echo Usage:
echo   build-native.bat
echo   build-native.bat release
goto :fail

:release_failed
echo.
echo ==========================================================
echo RELEASE PACKAGING FAILED
echo ==========================================================
echo The portable folder was built, but one or both GitHub release ZIPs could not be created.
if defined RELEASE_PORTABLE_ZIP if exist "%RELEASE_PORTABLE_ZIP%" del /f /q "%RELEASE_PORTABLE_ZIP%" >nul 2>nul
if defined RELEASE_SOURCE_ZIP if exist "%RELEASE_SOURCE_ZIP%" del /f /q "%RELEASE_SOURCE_ZIP%" >nul 2>nul
goto :fail

:no_msvc
echo.
echo ERROR: Visual Studio C++ Build Tools were not found.
echo Install the Desktop development with C++ workload.
goto :fail

:no_rc
echo.
echo ERROR: rc.exe was not found. The Windows SDK component is missing.
goto :fail

:env_failed
echo.
echo ERROR: Visual Studio was found but its x64 environment could not be loaded.
goto :fail

:ytdlp_download_failed
echo.
echo ERROR: Could not prepare the primary/authenticated yt-dlp backends under Tools\yt-dlp.
goto :package_failed

:ffmpeg_download_failed
echo.
echo ERROR: Could not prepare Tools\FFmpeg\bin\ffmpeg.exe from the Gyan release essentials package.
goto :package_failed

:package_failed
echo.
echo ==========================================================
echo PACKAGING FAILED
echo ==========================================================
if defined DIST if exist "%DIST%" rmdir /s /q "%DIST%" >nul 2>nul
goto :fail

:build_failed
echo.
echo ==========================================================
echo BUILD FAILED
echo ==========================================================
echo Review the compiler/resource message above.

:fail
if defined BUILD_WORK if exist "%BUILD_WORK%" rmdir /s /q "%BUILD_WORK%" >nul 2>nul
echo.
pause
exit /b 1
