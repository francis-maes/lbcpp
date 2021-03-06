INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MCGPLibrary.xml MCGPLibrary.cpp)

SET(MCGP_PROBLEM_SOURCES
  BooleanParityProblem.h
  BooleanMultiplexerProblem.h
  SymbolicRegressionProblem.h
  SantaFeTrailProblem.h
)

SET(MCGP_SOURCES
  precompiled.h
  precompiled.cpp
  TestDomain.h
  SampleExpressionTrajectories.h
  OptimizeExpressionSampler.h
  TreeGPOperations.h
  TreeGPSamplers.h
  ${MCGP_PROBLEM_SOURCES}
  MCGPSandBox.h
  RegressionSandBox.h
  ClassificationSandBox.h
  SurrogateQuarticSandBox.h
  MCGPLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MCGPLibrary.cpp
)

SOURCE_GROUP(Problems FILES ${MCGP_PROBLEM_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled MCGP_SOURCES)

ADD_LIBRARY(project-mcgp SHARED ${MCGP_SOURCES})
TARGET_LINK_LIBRARIES(project-mcgp ml)

