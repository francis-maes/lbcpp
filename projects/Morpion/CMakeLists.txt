INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MorpionLibrary.xml MorpionLibrary.cpp)

SET(LBCPP_MORPION_SOURCES
  precompiled.h
  precompiled.cpp
  MorpionBoard.h
  MorpionProblem.h
  MorpionSandBox.h
  MorpionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MorpionLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_MORPION_SOURCES)

ADD_LIBRARY(lbcpp-morpion SHARED ${LBCPP_MORPION_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-morpion lbcpp-ml)

