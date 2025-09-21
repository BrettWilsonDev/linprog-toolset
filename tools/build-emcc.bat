@echo off
pushd %~dp0

REM Create build directory if it doesn't exist
if not exist ..\build-em mkdir ..\build-em
cd ..\build-em

REM Configure with emcmake
IF "%1"=="debug" (
    call emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Debug
) ELSE IF NOT "%1"=="" (
    echo Unknown argument: %1
    exit /b 1
) ELSE (
    call emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
)

REM Build
call emmake make

REM Check if build succeeded
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM Move .js and .wasm to desired output directory, overwrite if exists
set OUTPUT_DIR=..\src\web\wasm
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

move /Y linprog-toolset.js %OUTPUT_DIR%
move /Y linprog-toolset.wasm %OUTPUT_DIR%

echo Build complete! Files moved to %OUTPUT_DIR%

popd
