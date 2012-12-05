INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MorpionLibrary.xml MorpionLibrary.cpp)

SET(MORPION_SOURCES
  precompiled.h
  precompiled.cpp
  MorpionBoard.h
  MorpionProblem.h
  MorpionSandBox.h
  MorpionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MorpionLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled MORPION_SOURCES)

ADD_LIBRARY(project-morpion SHARED ${MORPION_SOURCES})
TARGET_LINK_LIBRARIES(project-morpion ml)

