#!/bin/bash

set -x
set -e

ORGDIR=$(pwd)

### build choreonoid
cd ${ORGDIR}
export WKDIR=${ORGDIR}/all_build
export INSTALL_EM_DIR=${ORGDIR}/em_lib
export EMQT_BUILD=${ORGDIR}/emqt_build
mkdir -p ${WKDIR}

##zlib
cd ${WKDIR}
git clone https://github.com/madler/zlib.git build_zlib
cd ${WKDIR}/build_zlib
git checkout -b v1.3.1 v1.3.1
mkdir -p ${WKDIR}/build_zlib/build
emcmake cmake ${WKDIR}/build_zlib -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DCMAKE_C_FLAGS="${_MY_CFLAGS}" -DCMAKE_CXX_FLAGS="${_MY_CFLAGS}"
emmake make install -j$(nproc)
ln -s ${INSTALL_EM_DIR}/include ${INSTALL_EM_DIR}/include/../zlib-1.2.13

##libzip
cd ${WKDIR}
git clone https://github.com/nih-at/libzip.git build_libzip
cd ${WKDIR}/build_libzip
git checkout -b v1.9.2 v1.9.2
patch -p1 < ${ORGDIR}/emscripten_build/libzip.em.patch
mkdir -p ${WKDIR}/build_libzip/build; cd ${WKDIR}/build_libzip/build
emcmake cmake ${WKDIR}/build_libzip -DBUILD_TOOLS=OFF -DBUILD_REGRESS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOC=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS="-I${INSTALL_EM_DIR}/include ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR}
emmake make install -j$(nproc)

#
export PATH=$PATH:${ORGDIR}/qt_lib/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${ORGDIR}/qt_lib/lib

#
mkdir -p ${ORGDIR}/build
(cd ${ORGDIR}/build; cmake .. -DENABLE_FREE_TYPE=OFF) ## for making Util/Config.h

## apt gettext libzip-dev libeigen3-dev uuid-dev pkgconf

#
cd ${WKDIR}
mkdir -p ${WKDIR}/build_fmt; cd ${WKDIR}/build_fmt
emcmake cmake ${ORGDIR}/thirdparty/fmt-11.0.0 -DFMT_DOC=OFF -DFMT_TEST=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DCMAKE_C_FLAGS="${_MY_CFLAGS}" -DCMAKE_CXX_FLAGS="${_MY_CFLAGS}"
emmake make install -j$(nproc)

#
cd ${WKDIR}
mkdir -p ${WKDIR}/build_yaml; cd ${WKDIR}/build_yaml
emcmake cmake ${ORGDIR}/thirdparty/libyaml-0.2.5 -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DCMAKE_C_FLAGS="${_MY_CFLAGS}" -DCMAKE_CXX_FLAGS="${_MY_CFLAGS}"
emmake make -j$(nproc)
cp libyaml.a ${INSTALL_EM_DIR}/lib
cp -r ${ORGDIR}/thirdparty/libyaml-0.2.5/include/* ${INSTALL_EM_DIR}/include


cd ${WKDIR}
mkdir -p ${WKDIR}/build_jpeg; cd ${WKDIR}/build_jpeg
emcmake cmake ${ORGDIR}/thirdparty/jpeg-9c -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DCMAKE_C_FLAGS="${_MY_CFLAGS}" -DCMAKE_CXX_FLAGS="${_MY_CFLAGS}"
emmake make -j$(nproc)
cp libjpeg.a ${INSTALL_EM_DIR}/lib
(cd ${ORGDIR}/thirdparty/jpeg-9c; cp jpeglib.h jconfig.h jmorecfg.h ${INSTALL_EM_DIR}/include)


mkdir -p ${WKDIR}/build_png; cd ${WKDIR}/build_png
emcmake cmake ${ORGDIR}/thirdparty/lpng1232 -DCMAKE_C_FLAGS="-I${INSTALL_EM_DIR}/include ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR}
emmake make -j$(nproc)
cp libpng_cnoid.a ${INSTALL_EM_DIR}/lib/libpng.a
(cd ${ORGDIR}/thirdparty/lpng1232; cp png.h pngconf.h ${INSTALL_EM_DIR}/include)


## fas?_float
cp -r ${ORGDIR}/thirdparty/fast_float-0.8.0/include/fast_float ${INSTALL_EM_DIR}/include


#<Util>
(cp ${ORGDIR}/build/src/Util/Config.h ${ORGDIR}/src/Util ) ##
mkdir -p ${WKDIR}/build_util; cd ${WKDIR}/build_util
emcmake cmake ${ORGDIR}/src/Util -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
emmake make -j$(nproc)
cp libCnoidUtil.a ${INSTALL_EM_DIR}/lib


#<Body>
mkdir -p ${WKDIR}/build_body; cd ${WKDIR}/build_body
emcmake cmake ${ORGDIR}/src/Body -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
emmake make -j$(nproc)
cp libCnoidBody.a ${INSTALL_EM_DIR}/lib


#<GLSceneRender>
mkdir -p ${WKDIR}/build_glscenerender; cd ${WKDIR}/build_glscenerender
emcmake cmake ${ORGDIR}/src/GLSceneRenderer --toolchain=${EMQT_BUILD}/qtbase/lib/cmake/Qt6/qt.toolchain.cmake -DCMAKE_FIND_ROOT_PATH=${EMQT_BUILD}/qtbase/lib/cmake -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
emmake make -j$(nproc)
cp libCnoidGLSceneRenderer.a ${INSTALL_EM_DIR}/lib


#<Base>
mkdir -p ${WKDIR}/build_base; cd ${WKDIR}/build_base
emcmake cmake ${ORGDIR}/src/Base --toolchain=${EMQT_BUILD}/qtbase/lib/cmake/Qt6/qt.toolchain.cmake -DCMAKE_FIND_ROOT_PATH=${EMQT_BUILD}/qtbase/lib/cmake -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I${ORGDIR}/thirdparty/CLI11 -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
emmake make -j$(nproc)
cp libCnoidBase.a ${INSTALL_EM_DIR}/lib


#<BodyPlugin>
mkdir -p ${WKDIR}/build_bodyplugin; cd ${WKDIR}/build_bodyplugin
emcmake cmake ${ORGDIR}/src/BodyPlugin --toolchain=${EMQT_BUILD}/qtbase/lib/cmake/Qt6/qt.toolchain.cmake -DCMAKE_FIND_ROOT_PATH=${EMQT_BUILD}/qtbase/lib/cmake -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I${ORGDIR}/thirdparty/CLI11 -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
emmake make -j$(nproc)
cp libCnoidBodyPlugin.a ${INSTALL_EM_DIR}/lib


#<assembler>
#mkdir -p ${WKDIR}/build_assem; cd ${WKDIR}/build_assem
#( cd ${ORGDIR}/ext; git clone https://github.com/IRSL-tut/robot_assembler_plugin.git -b emscripten_build )
#emcmake cmake ${ORGDIR}/ext/robot_assembler_plugin -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR}
#emmake make
#cp libRobotAssembler.a ${INSTALL_EM_DIR}/lib


###
#emcc --bind testbody.cpp -std=c++17  -s WASM=1 -I${ORGDIR}/include -I${ORGDIR} -I/usr/include/eigen3 -I${INSTALL_EM_DIR}/include $(ls -1 ${INSTALL_EM_DIR}/lib/*.a) -fwasm-exceptions -o testbody.js

# ${EMQT_BUILD}/qtbase/bin/qt-cmake .. --toolchain=${EMQT_BUILD}/qtbase/lib/cmake/Qt6/qt.toolchain.cmake -DCMAKE_FIND_ROOT_PATH=${EMQT_BUILD}/qtbase/lib/cmake -DCMAKE_CXX_FLAGS="-I${INSTALL_EM_DIR}/include -I${ORGDIR}/include -I${ORGDIR} -I${ORGDIR}/thirdparty/CLI11 -I/usr/include/eigen3 -std=c++17 ${_MY_CFLAGS}" -DCMAKE_INSTALL_PREFIX=${INSTALL_EM_DIR} -DEMSCRIPTEN=1
