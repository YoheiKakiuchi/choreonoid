cmake_minimum_required(VERSION 3.16)
project(QtThreeJSSample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(QtThreeJSSample main.cpp)

# WebAssembly(Emscripten) 特有のヘッダーを使用するため
target_link_libraries(QtThreeJSSample PRIVATE Qt6::Widgets)
