INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(TestUnitsLibrary.xml TestUnitsLibrary.cpp)

SET(LBCPP_TEST_UNITS_SOURCES
  precompiled.h
  precompiled.cpp
  ExtraTreeTestUnit.h
  AllTestUnits.h
  TestUnitsLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/TestUnitsLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_TEST_UNITS_SOURCES)

ADD_LIBRARY(lbcpp-testunits${NAME_FILE_SUFFIX} SHARED ${LBCPP_TEST_UNITS_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-testunits${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})

