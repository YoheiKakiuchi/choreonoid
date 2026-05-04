#!/bin/bash

set -x
set -e

export HOST_QT=$(pwd)/qt_lib

ORGDIR=$(pwd)

## QT(SRC)
git clone https://github.com/qt/qt5.git qtsrc
(cd qtsrc; git checkout -b v6.8.3 v6.8.3)

## QT(HOST)
mkdir hostqt_build
cd hostqt_build
${ORGDIR}/qtsrc/configure -init-submodules -submodules qtbase,qttools,qtsvg,qtimageformats,qtnetworkauth,qtdeclarative -prefix ${HOST_QT} -shared

cmake --build . --parallel

cmake --install .

## emscripten
EM_VER=3.1.56
cd ${ORGDIR}
git clone https://github.com/emscripten-core/emsdk.git
(cd emsdk && ./emsdk install ${EM_VER} && ./emsdk activate ${EM_VER})
source emsdk/emsdk_env.sh

## QT(WASM)
mkdir emqt_build
cd emqt_build
../qtsrc/configure -qt-host-path $HOST_QT -platform wasm-emscripten -feature-wasm-simd128 -feature-wasm-exceptions -prefix $PWD/qtbase -submodules qtbase,qttools,qtdeclarative,qtsvg,qtimageformats
cmake --build . -t qtbase -t qtdeclarative -t qttools -t qtsvg -t qtimageformats --parallel
