#!/bin/bash

set -x
set -e

#qt_ver=6.8.3
#EM_VER=3.1.56
qt_ver=6.11.0
EM_VER=4.0.7

export HOST_QT=$(pwd)/Qt_${qt_ver}

_NUM_PARALLEL=
_CLANG_BUILD=
#_CLANG_BUILD='-DCMAKE_C_COMPILER=clang-20 -DCMAKE_CXX_COMPILER=clang++-20'

ORGDIR=$(pwd)

## QT(SRC)
if [ ! -e qtsrc_${qt_ver} ]; then
    git clone https://github.com/qt/qt5.git qtsrc_${qt_ver}
fi
(cd qtsrc_${qt_ver}; git checkout v${qt_ver})

## QT(HOST)
mkdir -p hostqt_build
cd hostqt_build

#${ORGDIR}/qtsrc/configure -init-submodules -submodules qtbase,qttools,qtsvg,qtimageformats,qtnetworkauth,qtdeclarative -prefix ${HOST_QT} -shared
${ORGDIR}/qtsrc_${qt_ver}/configure -init-submodules -submodules qtbase,qtdeclarative,qtimageformats,qtsvg,qt5compat,qtcharts,qtgraphs,qtwebsockets,qt3d,qtshadertools,qtwayland,qtnetworkauth,qttools -prefix ${HOST_QT} -shared ${_CLANG_BUILD}

cmake --build . --parallel ${_NUM_PARALLEL}

cmake --install .

## emscripten
cd ${ORGDIR}
if [ ! -e emsdk ]; then
    git clone https://github.com/emscripten-core/emsdk.git
fi
(cd emsdk && ./emsdk install ${EM_VER} && ./emsdk activate ${EM_VER})
source emsdk/emsdk_env.sh

## QT(WASM)
cd ${ORGDIR}
mkdir -p emqt_${qt_ver}
if [ -e emqt_build ]; then
    rm -rf emqt_build
fi
ln -s emqt_${qt_ver} emqt_build

cd emqt_${qt_ver}

${ORGDIR}/qtsrc_${qt_ver}/configure -qt-host-path $HOST_QT -platform wasm-emscripten -feature-opengles3 -feature-wasm-simd128 -prefix $(pwd)/qtbase -submodules qtbase,qtdeclarative,qtimageformats,qtsvg,qt5compat,qtcharts,qtgraphs,qtwebsockets,qttools

cmake --build . -t qtbase -t qtdeclarative -t qtimageformats -t qtsvg -t qt5compat -t qtcharts -t qtgraphs -t qtwebsockets -t qttools --parallel ${_NUM_PARALLEL}
