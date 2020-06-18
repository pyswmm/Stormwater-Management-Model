foreach ($PYBIN in C:\Python3?-x64\\python.exe) {
    Set-Location -Path $env:APPVEYOR_BUILD_FOLDER
    & $PYBIN -m pip install --upgrade pip
    & $PYBIN -m pip install scikit-build
    & $PYBIN -m pip wheel . --no-deps -w dist
}
