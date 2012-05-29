INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(Core/SequentialDecisionCoreLibrary.xml SequentialDecisionCoreLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Problem/SequentialDecisionProblemLibrary.xml SequentialDecisionProblemLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/SequentialDecisionWorkUnitLibrary.xml SequentialDecisionWorkUnitLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Go/SequentialDecisionGoLibrary.xml SequentialDecisionGoLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Bandits/SequentialDecisionBanditsLibrary.xml SequentialDecisionBanditsLibrary.cpp)
GENERATE_INTROSPECTION_CODE(GP/SequentialDecisionGPLibrary.xml SequentialDecisionGPLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SmallMDP/SmallMDPLibrary.xml SmallMDPLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Luape/LuapeSandBoxLibrary.xml LuapeSandBoxLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SequentialDecisionLibrary.xml SequentialDecisionLibrary.cpp)

SET(LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES
  Core/SearchTree.h
  Core/SearchTreeNode.h
  Core/SearchTree.cpp
  Core/SearchHeuristic.h
  Core/SearchPolicy.h
  Core/SearchFunction.h
  Core/SearchFunctionOnlineLearner.h
  Core/SearchTreeEvaluator.h
  Core/BeamSearchOptimizer.h
  Core/NestedMonteCarloOptimizer.h  
  Core/SinglePlayerMCTSOptimizer.h
  Core/SequentialDecisionCoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionCoreLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES
  Problem/LinearPointPhysicProblem.h
  Problem/DamienDecisionProblem.h
  Problem/LuaDecisionProblemState.h
  Problem/MountainCarProblem.h
  Problem/LeftOrRightControlProblem.h
  Problem/CarOnTheHillProblem.h
  Problem/AcrobotProblem.h
  Problem/BicycleBalancingProblem.h
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
  WorkUnit/InterpretHIVPolicy.h
  WorkUnit/LuapeEDASandBox.h
  WorkUnit/SequentialDecisionSandBox.h
  WorkUnit/GPSandBox.h
  WorkUnit/BBOptimization.h
  WorkUnit/AutoSGDSandBox.h
  WorkUnit/RFHooSandBox.h
  WorkUnit/EvaluateInterpretablePolicies.h
  WorkUnit/ExploChallengeSandBox.h
  WorkUnit/SequentialDecisionWorkUnitLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionWorkUnitLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_GP_SOURCES
  GP/GPExpression.h
  GP/GPExpression.cpp
  GP/GPExpressionSampler.h
  GP/GPExpressionBuilder.h
  GP/FormulaSearchProblem.h
  GP/GenerateUniqueFormulas.h
  GP/BanditFormulaOptimizer.h
  GP/PathsFormulaFeatureGenerator.h
  GP/OrdersFormulaFeatureGenerator.h
  GP/FormulaLearnAndSearch.h
  GP/SequentialDecisionGPLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionGPLibrary.cpp
)

SET(LBCPP_GP_SEARCH_PROBLEM_SOURCES
  GP/BanditFormulaSearchProblem.h
  GP/BanditNoParamSearchProblem.h
  GP/SmallMDPFormulaSearchProblem.h
  GP/LearningRuleFormulaSearchProblem.h
  GP/PolicyFormulaSearchProblem.h
  GP/ExploChallengeFormulaSearchProblem.h
)

SET(LBCPP_SEQUENTIAL_DECISION_BANDITS_SOURCES
  Bandits/DiscreteBanditDecisionProblem.h
  Bandits/DiscreteBanditPolicy.h
  Bandits/DiscreteBanditExperiment.h
  Bandits/CompareTunedBanditPoliciesExperiment.h
  Bandits/FindBanditsFormula.h
  Bandits/EpsilonGreedyDiscreteBanditPolicy.h
  Bandits/UCB1DiscreteBanditPolicy.h
  Bandits/UCB2DiscreteBanditPolicy.h
  Bandits/UCBvDiscreteBanditPolicy.h
  Bandits/KLUCBDiscreteBanditPolicy.h
  Bandits/PowerDiscreteBanditPolicy.h
  Bandits/GPExpressionDiscreteBanditPolicy.h
#  Bandits/BanditsMCTSWorkUnit.h
  Bandits/SequentialDecisionBanditsLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionBanditsLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_GO_SOURCES
  Go/GoProblem.h
  Go/GoProblem.cpp
  Go/LoadSGFFileFunction.h
  Go/GoRegionPerception.h
  Go/GoActionsPerception.h
  Go/GoSupervisedEpisode.h
  Go/GoActionScoringEvaluator.h
  Go/GoSandBox.h
  Go/GoBoostingSandBox.h
  Go/GoBoostingSandBox2.h
  Go/GoPredictWorkUnit.h
  Go/GoSGFToGTPWorkUnit.h
  Go/GoInterface.h
  Go/GoInterface.cpp
  Go/SequentialDecisionGoLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionGoLibrary.cpp
)

SET(LBCPP_SEQUENTIAL_DECISION_GO_PERCEPTION_SOURCES
  Go/Perception/GoStatePerception.h
  Go/Perception/GoRegionPerception.h
  Go/Perception/GoBoardPerception.h
  Go/Perception/GoBoardPositionPerception.h
)

SET(LBCPP_SEQUENTIAL_DECISION_LUAPE_SOURCES
  Luape/EmpiricalContinuousDistribution.h
  Luape/LuapeSoftStump.h
  Luape/LuapeSandBox.h
  Luape/LuapeClassificationSandBox.h
  Luape/MCObjective.h
  Luape/MCAlgorithm.h
  Luape/MCOptimizer.h
  Luape/MetaMCSandBox.h
  Luape/LuapeFormulaDiscovery.h
  Luape/LuapeBanditFormulaSearchProblem.h
  Luape/LuapeLearningFormulaSearchProblem.h
  Luape/LuapeLearningFormulaSearchProblem2.h
  Luape/LuapeSandBoxLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuapeSandBoxLibrary.cpp
)

SET(LBCPP_LUAPE_VRAC_SOURCES
  Luape/BanditPoolWeakLearner.h
  Luape/BanditPoolWeakLearner.cpp
  Luape/ProductWeakLearner.h
)

SET(LBCPP_SEQUENTIAL_DECISION_SMALL_MDP_SOURCES
  SmallMDP/SmallMDP.h
  SmallMDP/SmallMDPPolicy.h
  SmallMDP/SmallMDPPolicy.cpp
  SmallMDP/ParameterizedSmallMDPPolicy.h
  SmallMDP/SmallMDPSandBox.h
  SmallMDP/EvaluateSmallMDPPolicies.h
  SmallMDP/SmallMDPLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SmallMDPLibrary.cpp
)
  

SET(LBCPP_SEQUENTIAL_DECISION_SOURCES
  precompiled.h
  precompiled.cpp
  
  ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES}
    ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_GO_SOURCES}
    ${LBCPP_SEQUENTIAL_DECISION_GO_PERCEPTION_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_GP_SOURCES}
    ${LBCPP_GP_SEARCH_PROBLEM_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_BANDITS_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_LUAPE_SOURCES}
    ${LBCPP_LUAPE_VRAC_SOURCES}
  ${LBCPP_SEQUENTIAL_DECISION_SMALL_MDP_SOURCES}

  SequentialDecisionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SequentialDecisionLibrary.cpp
)

SOURCE_GROUP(Core FILES ${LBCPP_SEQUENTIAL_DECISION_CORE_SOURCES})
SOURCE_GROUP(Problem FILES ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_SOURCES})
SOURCE_SUB_GROUP(Problem Damien FILES ${LBCPP_SEQUENTIAL_DECISION_PROBLEM_DAMIEN_SOURCES})

SOURCE_GROUP(WorkUnit FILES ${LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SOURCES})
SOURCE_GROUP(Go FILES ${LBCPP_SEQUENTIAL_DECISION_GO_SOURCES})
SOURCE_SUB_GROUP(Go Perception FILES ${LBCPP_SEQUENTIAL_DECISION_GO_PERCEPTION_SOURCES})

SOURCE_GROUP(GP FILES ${LBCPP_SEQUENTIAL_DECISION_GP_SOURCES})
SOURCE_SUB_GROUP(GP SearchProblem FILES ${LBCPP_GP_SEARCH_PROBLEM_SOURCES})

SOURCE_GROUP(Bandits FILES ${LBCPP_SEQUENTIAL_DECISION_BANDITS_SOURCES})
SOURCE_GROUP(Luape FILES ${LBCPP_SEQUENTIAL_DECISION_LUAPE_SOURCES})
SOURCE_SUB_GROUP(Luape Vrac FILES ${LBCPP_LUAPE_VRAC_SOURCES})

SOURCE_GROUP(SmallMDP FILES ${LBCPP_SEQUENTIAL_DECISION_SMALL_MDP_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SEQUENTIAL_DECISION_SOURCES)

ADD_LIBRARY(sequentialdecision${NAME_FILE_SUFFIX} SHARED ${LBCPP_SEQUENTIAL_DECISION_SOURCES})
TARGET_LINK_LIBRARIES(sequentialdecision${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})
