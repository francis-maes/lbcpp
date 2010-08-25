INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

MACRO(ADD_CPP_EXAMPLE CATEGORY FILENAME)
  SET(NAME ${CATEGORY}-${FILENAME})
  ADD_EXECUTABLE(${NAME} ${CATEGORY}/${FILENAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CPP_EXAMPLE)

ADD_CPP_EXAMPLE(Misc UserTypeTraits)
ADD_CPP_EXAMPLE(Misc MissingValues)

GENERATE_INTROSPECTION_CODE(Misc/SimpleIntrospection.xml SimpleIntrospection.cpp)
ADD_EXECUTABLE(SimpleIntrospection Misc/SimpleIntrospection.xml ${CMAKE_CURRENT_BINARY_DIR}/SimpleIntrospection.cpp)
TARGET_LINK_LIBRARIES(SimpleIntrospection lbcpp-core)
