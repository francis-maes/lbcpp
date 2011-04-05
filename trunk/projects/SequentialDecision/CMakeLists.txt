INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(Core/SequentialDecisionCoreLibrary.xml SequentialDecisionCoreLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Problem/SequentialDecisionProblemLibrary.xml SequentialDecisionProblemLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/SequentialDecisionWorkUnitLibrary.xml SequentialDecisionWorkUnitLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Go/SequentialDecisionGoLibrary.xml SequentialDecisionGoLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SequentialDecisionLibrary.xml SequentialDecisionLibrary.cpp)

SET(LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES
  Core/DecisionProblem.h
  Core/DecisionProblem.cpp
  Core/Policy.h
  Core/SearchTree.h
  Core/SearchTreeNode.h
  Core/SearchTree.cpp
  Core/SearchHeuristic.h
  Core/SearchPolicy.h
  Core/SearchFunction.h
  Core/SearchFunctionOnlineLearner.h
  Core/SearchTreeEvaluator.h
  Core/SequentialDecisionCoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionCoreLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES
  Problem/LinearPointPhysicProblem.h
  Problem/DamienDecisionProblem.h
  Problem/SequentialDecisionProblemLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionProblemLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_SOURCES
  Problem/Damien/optimalControlProblem.hpp
  Problem/Damien/invertedPendulum.hpp
  Problem/Damien/HIV.hpp
)

SET(LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES
  WorkUnit/HIVSandBox.h
  WorkUnit/SequentialDecisionSandBox.h
  WorkUnit/SequentialDecisionWorkUnitLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionWorkUnitLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_GO_SOURCES
  Go/GoProblem.h
  Go/GoProblem.cpp
  Go/LoadSGFFileFunction.h
  Go/GoActionsPerception.h
  Go/GoSupervisedEpisode.h
  Go/GoSandBox.h
  Go/GoPredictWorkUnit.h
  Go/GoInterface.h
  Go/GoInterface.cpp
  Go/SequentialDecisionGoLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionGoLibrary.cpp
)


SET(LBCPP_SEQUENTIAL_DECISION_SOURCES
  precompiled.h
  precompiled.cpp
  
  ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_GO_SOURCES}

  SequentialDecisionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionLibrary.cpp
)

SOURCE_GROUP(Core FILES ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES})
SOURCE_GROUP(Problem FILES ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES})
SOURCE_SUB_GROUP(Problem Damien FILES ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_SOURCES})

SOURCE_GROUP(WorkUnit FILES ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES})
SOURCE_GROUP(Go FILES ${LBCPP_SEQUENTIAL_DECISION_GO_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SEQUENTIAL_DECISION_SOURCES)

ADD_LIBRARY(sequentialdecision${NAME_FILE_SUFFIX} SHARED ${LBCPP_SEQUENTIAL_DECISION_SOURCES})
TARGET_LINK_LIBRARIES(sequentialdecision${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})
