INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include  
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(ExamplesLibrary.xml ExamplesLibrary.cpp)

SET(LBCPP_EXAMPLES_SOURCES
  precompiled.h
  precompiled.cpp
  MissingValuesExample.h
  SimpleIntrospectionExample.h
  WorkUnitExample.h
  RandomGeneratorExample.h
  ExamplesLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExamplesLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_EXAMPLES_SOURCES)

ADD_LIBRARY(lbcpp-examples SHARED ${LBCPP_EXAMPLES_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-examples lbcpp-ml)

