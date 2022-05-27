cmake_minimum_required(VERSION 3.10)

#project(choreonoidClient LANGUAGES C++)
project(choreonoidClient)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network Concurrent REQUIRED)

add_executable(choreonoidClient cnoid_client.cpp ClientSocket.h)
target_link_libraries(choreonoidClient Qt5::Core Qt5::Widgets Qt5::Concurrent Qt5::Network readline history)


