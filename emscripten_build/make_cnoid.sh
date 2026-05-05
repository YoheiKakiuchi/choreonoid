#!/bin/bash

set -x
set -e

ORGDIR=$(pwd)

### build choreonoid
cd ${ORGDIR}
export WKDIR=${ORGDIR}/all_build
export INSTALL_EM_DIR=${ORGDIR}/em_lib
mkdir -p ${WKDIR}

##zlib
cd ${WKDIR}
git clone https://github.com/madler/zlib.git build_zlib
cd ${WKDIR}/build_zlib
git checkout -b v1.3.1 v1.3.1
mkdir -p ${WKDIR}/build_zlib/build
emcmake cmake ${WKDIR}/build_zlib -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DCMAKE_C_FLAGS="${_MY_CFLAGS}" -DCMAKE_CXX_FLAGS="${_MY_CFLAGS}"
emmake make install

##libzip
cd ${WKDIR}
git clone https://github.com/nih-at/libzip.git build_libzip
cd ${WKDIR}/build_libzip
git checkout -b v1.9.2 v1.9.2
patch -p1 < ${CURDIR}/libzip.em.patch
mkdir -p ${WKDIR}/build_libzip/build; cd ${WKDIR}/build_libzip/build
emcmake cmake ${WKDIR}/build_libzip -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS="-I${INSTALL_EM_DIR}/include ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR}
emmake make install

