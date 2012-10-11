INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MCGPLibrary.xml MCGPLibrary.cpp)

SET(LBCPP_MCGP_SOURCES
  precompiled.h
  precompiled.cpp
  TestDomain.h
  BooleanParityProblem.h
  BooleanMultiplexerProblem.h
  F8SymbolicRegressionProblem.h
  SantaFeTrailProblem.h
  MCGPSandBox.h
  MCGPLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MCGPLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_MCGP_SOURCES)

ADD_LIBRARY(lbcpp-mcgp SHARED ${LBCPP_MCGP_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-mcgp lbcpp-ml)
