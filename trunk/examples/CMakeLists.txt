INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

MACRO(ADD_CPP_EXAMPLE CATEGORY FILENAME)
  SET(NAME ${CATEGORY}-${FILENAME})
  ADD_EXECUTABLE(${NAME} ${CATEGORY}/${FILENAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CPP_EXAMPLE)

ADD_CPP_EXAMPLE(Misc MissingValues)
ADD_CPP_EXAMPLE(Misc XorRegression)
ADD_CPP_EXAMPLE(Misc FeatureGenerators)
ADD_CPP_EXAMPLE(Misc UserInterfaceExecutionCallback)

ADD_CPP_EXAMPLE(TestUnit TestUnit)

GENERATE_INTROSPECTION_CODE(Misc/SimpleIntrospection.xml SimpleIntrospection.cpp)
ADD_EXECUTABLE(SimpleIntrospection Misc/SimpleIntrospection.xml ${CMAKE_CURRENT_BINARY_DIR}/SimpleIntrospection.cpp)
TARGET_LINK_LIBRARIES(SimpleIntrospection lbcpp-core)
