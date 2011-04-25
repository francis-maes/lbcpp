INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(ExamplesLibrary.xml ExamplesLibrary.cpp)

SET(LBCPP_EXAMPLES_SOURCES
  precompiled.h
  precompiled.cpp
  FeatureGeneratorsExample.h
  MissingValuesExample.h
  SimpleIntrospectionExample.h
  WorkUnitExample.h
  XorRegressionExample.h
  TrainTestLearningMachine.h
  GeneticProgrammingExample.h
  OptimizerExampleWorkUnit.h
  RandomGeneratorExample.h
  ExamplesLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExamplesLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_EXAMPLES_SOURCES)

ADD_LIBRARY(lbcpp-examples${NAME_FILE_SUFFIX} SHARED ${LBCPP_EXAMPLES_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-examples${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})

