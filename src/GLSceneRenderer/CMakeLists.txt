if(${EMSCRIPTEN})
  cmake_minimum_required(VERSION 3.15)
  project(emscripten_build)
  include(CMakeLists.em.txt)
else()

set(sources
  GLSceneRenderer.cpp
  GL1SceneRenderer.cpp
  GLSLProgram.cpp
  ShaderPrograms.cpp
  GLSLSceneRenderer.cpp
  #gl_core_3_3.c
  #gl_core_4_0.c
  #gl_core_4_4.c
  gl_1_5.c
  )

set(headers
  GLSceneRenderer.h
  GL1SceneRenderer.h
  GLSLProgram.h
  ShaderPrograms.h
  GLSLSceneRenderer.h
  exportdecl.h
  )

set(target CnoidGLSceneRenderer)

choreonoid_qt_add_resources(RC_SRCS GLSceneRenderer.qrc)

make_gettext_mofiles(${target} mofiles)

message("CNOID_QT : ${CHOREONOID_QT_CORE_LIBRARY}")

find_package(Qt6 REQUIRED COMPONENTS Gui)

set(libraries PUBLIC CnoidUtil PRIVATE ${CHOREONOID_QT_CORE_LIBRARY} Qt6::Gui)

if(MSVC)
  set(libraries ${libraries} PUBLIC general glu32 opengl32)
else()
  set(libraries ${libraries} PUBLIC OpenGL::GLU)
endif()

if(CNOID_ENABLE_FREE_TYPE)
  set(sources ${sources} GLFreeType.cpp)
  if(MSVC)
    set(libraries ${libraries} PRIVATE Freetype::Freetype)
  else()
    set(libraries ${libraries} PRIVATE PkgConfig::FreeType2)
  endif()
endif()

choreonoid_add_library(${target} SHARED ${sources} ${mofiles} ${RC_SRCS} HEADERS ${headers})
target_link_libraries(${target} ${libraries})

endif(${EMSCRIPTEN})
