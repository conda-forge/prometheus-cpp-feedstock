@echo on
set "PROMETHEUS_TESTS=OFF"
if "%target_platform%" == "win-arm64" set "PROMETHEUS_TESTS=ON"

mkdir build-cpp
if errorlevel 1 exit 1

cd build-cpp
cmake .. ^
    -GNinja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH=%CONDA_PREFIX% ^
    -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% ^
    -DBUILD_SHARED_LIBS=ON ^
    -DENABLE_PUSH=ON ^
    -DENABLE_COMPRESSION=ON ^
    -DENABLE_TESTING=%PROMETHEUS_TESTS%
if errorlevel 1 exit /b 1

cmake --build . --parallel 4
if errorlevel 1 exit 1

REM install the libraries and headers
cmake --install .
if errorlevel 1 exit /b 1

if "%target_platform%" == "win-arm64" (
    set "PATH=%LIBRARY_BIN%;%PATH%"
    ctest --output-on-failure -C Release
    if errorlevel 1 exit /b 1
)
