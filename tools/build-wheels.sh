#!/bin/bash
set -e -u -x

function repair_wheel {
    wheel="$1"
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel" --plat "$PLAT" -w /io/wheelhouse/
    fi
}

# Install a system package required by our library
# yum install -y xxxx

# Compile wheels
for PYBIN in /opt/python/*/bin; do
    echo "${PYBIN}"
    "${PYBIN}/pip" install scikit-build
    "${PYBIN}/pip" wheel /io/ --no-deps -w wheelhouse/
done

# Bundle external shared libraries into the wheels
for whl in wheelhouse/*.whl; do
    repair_wheel "$whl"
done

# Install packages and test
for PYBIN in /opt/python/*/bin/; do
    "${PYBIN}/pip" install swmm --no-index -f /io/wheelhouse --prefix .
    "${PYBIN}/pip" show -f swmm
    (cd "$HOME"; "./bin/run-swmm --version")
done
