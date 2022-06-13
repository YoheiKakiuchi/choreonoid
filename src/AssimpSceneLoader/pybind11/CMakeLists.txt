set(headers
  PyAssimp.h
  )

choreonoid_add_library(CnoidPyAssimp SHARED PyAssimp.cpp HEADERS ${headers}) # dummy for win
target_link_libraries(CnoidPyAssimp ${PYTHON_LIBRARIES}) # dummy for win

choreonoid_add_python_module(PyAssimp
  PyAssimpModule.cpp
  )

target_link_libraries(PyAssimp CnoidAssimpSceneLoader) ## assimp loader
