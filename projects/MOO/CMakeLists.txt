INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/extern/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LBCpp_SOURCE_DIR}/extern/rlglue
)

GENERATE_INTROSPECTION_CODE(MOOLibrary.xml MOOLibrary.cpp)

SET(MOO_SOURCES
  precompiled.h
  precompiled.cpp
  DiscoveryScience.h
  HoeffdingTreeLearner.h
  HoeffdingTreeLearner.cpp
 # HoeffdingTreeLearnerTest.h
  RewardLearningTest.h
  SolverInfo.h
  SharkProblems.h
  FQIBasedSolver.h
  TestRandomForestUncertainties.h
  MOOSandBox.h
  TestSBOInnerOptimizers.h
  SBOExperiments.h
  SBOEnsembleSizeExperiments.h
  PerceptronTest.h
  LinearRegressionTests.h
  HoeffdingTreeLearnerExperiments.h
  WFGProblems.h
#  DTLZFronts.cpp
  MOOLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MOOLibrary.cpp
)

GENERATE_LBCPP_PRECOMPILED(precompiled MOO_SOURCES)

ADD_LIBRARY(project-moo SHARED ${MOO_SOURCES})
TARGET_LINK_LIBRARIES(project-moo ml rlglue)

