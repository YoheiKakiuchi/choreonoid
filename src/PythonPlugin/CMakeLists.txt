if(NOT ENABLE_PYTHON OR NOT ENABLE_GUI)
  return()
endif()

set(target CnoidPythonPlugin)

set(sources
  PythonPlugin.cpp
  PythonExecutor.cpp
  PythonScriptItem.cpp
  PythonScriptItemImpl.cpp
  PythonConsoleView.cpp
  )

set(headers
  PythonExecutor.h
  PythonScriptItem.h
  PythonScriptItemImpl.h
  Readline.h
  )

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(sources ${sources} Readline.cpp)
  set(qtheaders Readline.h)
  QT5_WRAP_CPP(qtsources ${qtheaders})
  set(sources ${sources} ${qtsources})
endif()

choreonoid_make_gettext_mo_files(${target} mofiles)
choreonoid_add_plugin(${target} ${sources} ${mofiles} HEADERS ${headers})
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_link_libraries(${target} CnoidPyUtil readline)
else()
  target_link_libraries(${target} CnoidPyUtil )
endif()

configure_file(rbimporter.py ${PROJECT_BINARY_DIR}/${CHOREONOID_PYTHON_SUBDIR}/cnoid/ COPYONLY)
install(FILES rbimporter.py DESTINATION ${CHOREONOID_PYTHON_SUBDIR}/cnoid)

add_subdirectory(pybind11)
