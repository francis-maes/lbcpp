INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include  
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MOOLibrary.xml MOOLibrary.cpp)

SET(LBCPP_MOO_SOURCES
  precompiled.h
  precompiled.cpp
  SharkProblems.h
  FQIBasedSolver.h
  MOOSandBox.h
  MOOLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MOOLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_MOO_SOURCES)

ADD_LIBRARY(lbcpp-moo SHARED ${LBCPP_MOO_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-moo lbcpp-ml)

