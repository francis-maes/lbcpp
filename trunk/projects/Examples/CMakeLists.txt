INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(ExamplesLibrary.xml ExamplesLibrary.cpp)

SET(LBCPP_EXAMPLES_SOURCES
  FeatureGeneratorsExample.h
  MissingValuesExample.h
  SimpleIntrospectionExample.h
  WorkUnitExample.h
  XorRegressionExample.h
  TrainTestLearningMachine.h
  GeneticProgrammingExample.h
  ExamplesLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExamplesLibrary.cpp
)

ADD_LIBRARY(lbcpp-examples SHARED ${LBCPP_EXAMPLES_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-examples lbcpp-core)

