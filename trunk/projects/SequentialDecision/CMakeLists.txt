INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(Core/SequentialDecisionCoreLibrary.xml SequentialDecisionCoreLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Problem/SequentialDecisionProblemLibrary.xml SequentialDecisionProblemLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/SequentialDecisionWorkUnitLibrary.xml SequentialDecisionWorkUnitLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SequentialDecisionLibrary.xml SequentialDecisionLibrary.cpp)

SET(LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES
  Core/SequentialDecisionProblem.h
  Core/SequentialDecisionProblem.cpp
  Core/SearchSpace.h
  Core/SearchSpace.cpp
  Core/SearchHeuristic.h
  Core/SequentialDecisionCoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionCoreLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES
  Problem/LinearPointPhysicProblem.h
  Problem/SequentialDecisionProblemLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionProblemLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES
  WorkUnit/SequentialDecisionSandBox.h
  WorkUnit/SequentialDecisionWorkUnitLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionWorkUnitLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_SOURCES
  ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES}

  SequentialDecisionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionLibrary.cpp
)

SOURCE_GROUP(Core FILES ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES})
SOURCE_GROUP(Problem FILES ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES})
SOURCE_GROUP(WorkUnit FILES ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES})

ADD_LIBRARY(sequentialdecision${NAME_FILE_SUFFIX} SHARED ${LBCPP_SEQUENTIAL_DECISION_SOURCES})
TARGET_LINK_LIBRARIES(sequentialdecision${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})
