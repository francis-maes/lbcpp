INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MCGPLibrary.xml MCGPLibrary.cpp)

SET(LBCPP_MCGP_PROBLEM_SOURCES
  BooleanParityProblem.h
  BooleanMultiplexerProblem.h
  F8SymbolicRegressionProblem.h
  SantaFeTrailProblem.h
)

SET(LBCPP_MCGP_SOURCES
  precompiled.h
  precompiled.cpp
  TestDomain.h
  SampleExpressionTrajectories.h
  ${LBCPP_MCGP_PROBLEM_SOURCES}
  MCGPSandBox.h
  MCGPLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MCGPLibrary.cpp
)

SOURCE_GROUP(Problems FILES ${LBCPP_MCGP_PROBLEM_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_MCGP_SOURCES)

ADD_LIBRARY(lbcpp-mcgp SHARED ${LBCPP_MCGP_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-mcgp lbcpp-ml)

