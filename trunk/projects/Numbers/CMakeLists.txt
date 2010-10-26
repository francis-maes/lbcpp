INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(NumbersClasses.xml NumbersClasses.cpp)

SET(LBCPP_NUMBERS_SOURCES
  NumbersSandBox.cpp  
  NumberPerceptions.h
  NumbersClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumbersClasses.cpp
)

ADD_EXECUTABLE(numbers ${LBCPP_NUMBERS_SOURCES})
TARGET_LINK_LIBRARIES(numbers lbcpp-core)


