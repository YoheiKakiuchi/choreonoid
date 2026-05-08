cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0057 NEW)

project(FileDialogTest C CXX)

set(sources
  filemain.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(filemain ${sources})

target_link_libraries(filemain PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
