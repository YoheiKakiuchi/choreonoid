cmake_minimum_required(VERSION 3.15)

if(${EMSCRIPTEN})
  include(CMakeLists.em.txt)
else()
  #include(CMakeLists.org.txt)
  include(CMakeLists.em.txt)
endif(${EMSCRIPTEN})
