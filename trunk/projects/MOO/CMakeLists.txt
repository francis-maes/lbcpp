INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/extern/shark/include  
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MOOLibrary.xml MOOLibrary.cpp)

SET(MOO_SOURCES
  precompiled.h
  precompiled.cpp
  SolverInfo.h
  SharkProblems.h
  FQIBasedSolver.h
  ExtraTreeIncrementalLearner.h
  MOOSandBox.h
  TestRandomForestUncertainties.h
  SBOExperiments.h
  MOOLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MOOLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled MOO_SOURCES)

ADD_LIBRARY(project-moo SHARED ${MOO_SOURCES})
TARGET_LINK_LIBRARIES(project-moo ml)

