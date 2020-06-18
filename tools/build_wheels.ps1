Set-Location -Path $env:APPVEYOR_BUILD_FOLDER
$files = Get-ChildItem -Path C:\Python3?-x64\* -Include python.exe
foreach ($PYBIN in $files) {
    & $PYBIN -m pip install --upgrade pip
    & $PYBIN -m pip install scikit-build
    & $PYBIN -m pip wheel . --no-deps -w dist
}