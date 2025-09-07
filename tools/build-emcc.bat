@echo off

pushd %~dp0

mkdir ..\build-em

cd ..\build-em

IF "%1"=="debug" (
    call emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Debug
) ELSE IF NOT "%1"=="" (
    echo Unknown argument: %1
    exit /b 1
) ELSE (
    call emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
)

call emmake make