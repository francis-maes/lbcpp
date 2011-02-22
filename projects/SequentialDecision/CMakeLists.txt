INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(System/SequentialDecisionSystemLibrary.xml SequentialDecisionSystemLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/SequentialDecisionWorkUnitLibrary.xml SequentialDecisionWorkUnitLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SequentialDecisionLibrary.xml SequentialDecisionLibrary.cpp)

SET(LBCPP_SEQUENTIAL_DECISION_SYSTEM_SOURCES
  System/SequentialDecisionSystem.h
  System/SequentialDecisionSystem.cpp
  System/LinearPointPhysicSystem.h
  System/SequentialDecisionSystemLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionSystemLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES
  WorkUnit/SequentialDecisionSandBox.h
  WorkUnit/SequentialDecisionWorkUnitLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionWorkUnitLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_SOURCES
  ${LBCPP_SEQUENTIAL_DECISION_SYSTEM_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES}

  SequentialDecisionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionLibrary.cpp
)

SOURCE_GROUP(System FILES ${LBCPP_SEQUENTIAL_DECISION_SYSTEM_SOURCES})
SOURCE_GROUP(WorkUnit FILES ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES})

ADD_LIBRARY(sequentialdecision${NAME_FILE_SUFFIX} SHARED ${LBCPP_SEQUENTIAL_DECISION_SOURCES})
TARGET_LINK_LIBRARIES(sequentialdecision${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})
