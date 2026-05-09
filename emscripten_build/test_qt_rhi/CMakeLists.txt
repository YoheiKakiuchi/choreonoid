cmake_minimum_required(VERSION 3.16)
project(QtRhiSample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ShaderTools を追加
find_package(Qt6 REQUIRED COMPONENTS Widgets ShaderTools)

qt_standard_project_setup()

qt_add_executable(QtRhiSample main.cpp)

target_link_libraries(QtRhiSample PRIVATE Qt6::Core Qt6::Gui Qt6::GuiPrivate Qt6::Widgets)

# シェーダーをコンパイルし、アプリのリソース (:/) に組み込む
qt6_add_shaders(QtRhiSample "rhi_shaders"
    PREFIX "/"
    FILES
        color.vert
        color.frag
)
