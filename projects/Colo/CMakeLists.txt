FIND_PACKAGE(JNI REQUIRED)

INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${JAVA_INCLUDE_PATH}
  ${JAVA_INCLUDE_PATH2}
)

GENERATE_INTROSPECTION_CODE(ColoLibrary.xml ColoLibrary.cpp)

SET(LBCPP_COLO_SOURCES
  precompiled.h
  precompiled.cpp
  ColoJavaWrapper.cpp
  ColoProblem.h
  SurrogateBasedColoSolver.h
  ColoSandBox.h
  ColoLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ColoLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_COLO_SOURCES)

ADD_LIBRARY(project-colo SHARED ${LBCPP_COLO_SOURCES})
TARGET_LINK_LIBRARIES(project-colo ml ${JAVA_JVM_LIBRARY} ${JAVA_AWT_LIBRARY})

