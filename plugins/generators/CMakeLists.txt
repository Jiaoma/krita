add_subdirectory(solid)
add_subdirectory(pattern)
add_subdirectory(simplexnoise)
add_subdirectory(screentone)
add_subdirectory(multigridpattern)
if(SeExpr2_FOUND)
  add_subdirectory(seexpr)
endif()
