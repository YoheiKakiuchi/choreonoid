set(libsources
  TestUtil.cpp
  )
set(headers
  TestUtil.h
  exportdecl.h
  )
set(sources
  TestPlugin.cpp
  TestBody.cpp
  )
set(headers
  TestBody.h
  exportdecl.h
  )

set(target CnoidTestPlugin)
set(targetlib Test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

choreonoid_add_library(${targetlib} SHARED ${libsources} HEADERS ${libheaders})
target_link_libraries(${targetlib} PUBLIC CnoidUtil)

choreonoid_add_plugin(${target} ${sources} HEADERS ${headers})
target_link_libraries(${target} PUBLIC CnoidBody ${targetlib})
