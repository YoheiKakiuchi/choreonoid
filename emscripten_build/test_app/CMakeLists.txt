cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0057 NEW)

project(CnoidWasm C CXX)

set(target CnoidWasm)

### only for linux
set(sources
  main.cpp
)
if (NOT WIN32)
  add_definitions(-Wno-deprecated-declarations)
endif()

#Qt
#find_package(Threads)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(${target} main.cpp)

target_link_directories(${target} PUBLIC /userdir/em_lib/lib)
target_link_libraries(${target} PRIVATE Qt6::OpenGLWidgets PUBLIC fmt yaml CnoidUtil CnoidBody CnoidGLSceneRenderer CnoidBase)
target_link_options(${target} PUBLIC -sFULL_ES3)
#target_link_libraries( render Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets CnoidUtil CnoidGLSceneRenderer)
#Qt6::Widgets Qt6::Gui Qt6::Core Qt6::OpenGLWidgets
