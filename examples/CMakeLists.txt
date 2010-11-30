INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

MACRO(ADD_CPP_EXAMPLE CATEGORY FILENAME)
  SET(NAME ${CATEGORY}-${FILENAME})
  ADD_EXECUTABLE(${NAME} ${CATEGORY}/${FILENAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CPP_EXAMPLE)

ADD_CPP_EXAMPLE(Misc MissingValues)
ADD_CPP_EXAMPLE(Misc XorRegression)
ADD_CPP_EXAMPLE(Misc FeatureGenerators)
ADD_CPP_EXAMPLE(Misc UserInterfaceExecutionCallback)

GENERATE_INTROSPECTION_CODE(TestUnit/TestUnitClasses.xml TestUnitClasses.cpp)

SET(TEST_UNIT_SOURCES
  TestUnit/MainWorkUnit.cpp
  TestUnit/ExtraTreeTestUnit.h
  TestUnit/TestUnitClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/TestUnitClasses.cpp
)

ADD_EXECUTABLE(MainTestUnit ${TEST_UNIT_SOURCES})
TARGET_LINK_LIBRARIES(MainTestUnit lbcpp-core)

GENERATE_INTROSPECTION_CODE(Misc/SimpleIntrospection.xml SimpleIntrospection.cpp)
ADD_EXECUTABLE(SimpleIntrospection Misc/SimpleIntrospection.xml ${CMAKE_CURRENT_BINARY_DIR}/SimpleIntrospection.cpp)
TARGET_LINK_LIBRARIES(SimpleIntrospection lbcpp-core)
