INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppMLLibrary.xml LBCppMLLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Comparator/ComparatorLibrary.xml ComparatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/SamplerLibrary.xml SamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Solver/SolverLibrary.xml SolverLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Aggregator/AggregatorLibrary.xml AggregatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Expression/ExpressionLibrary.xml ExpressionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Search/SearchLibrary.xml SearchLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Loader/LoaderLibrary.xml LoaderLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SplittingCriterion/SplittingCriterionLibrary.xml SplittingCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learner/LearnerLibrary.xml LearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

SET(ML_INCLUDES ../../include/ml)

SET(LBCPP_ML_INCLUDES 
  ${ML_INCLUDES}/predeclarations.h
  ${ML_INCLUDES}/Domain.h
  ${ML_INCLUDES}/Objective.h
  ${ML_INCLUDES}/Fitness.h
  ${ML_INCLUDES}/SolutionContainer.h
  ${ML_INCLUDES}/SolutionComparator.h
  ${ML_INCLUDES}/Problem.h
  ${ML_INCLUDES}/Sampler.h
  ${ML_INCLUDES}/Perturbator.h
  ${ML_INCLUDES}/Solver.h
  ${ML_INCLUDES}/Search.h
  ${ML_INCLUDES}/Function.h
  ${ML_INCLUDES}/Aggregator.h
  ${ML_INCLUDES}/Expression.h
  ${ML_INCLUDES}/PostfixExpression.h
  ${ML_INCLUDES}/ExpressionDomain.h
  ${ML_INCLUDES}/ExpressionSampler.h
  ${ML_INCLUDES}/BanditPool.h
  ${ML_INCLUDES}/SplittingCriterion.h
  ${ML_INCLUDES}/DoubleVector.h
  ${ML_INCLUDES}/RandomVariable.h  
  ${ML_INCLUDES}/IndexSet.h
  ${ML_INCLUDES}/BinaryKey.h
  ${ML_INCLUDES}/BinaryConfusionMatrix.h
  ${ML_INCLUDES}/IterationFunction.h
)


SET(LBCPP_ML_SOLVER_SOURCES
  Solver/Solver.cpp
  Solver/RandomSolver.h
  Solver/NRPASolver.h
  Solver/BeamNRPASolver.h
  Solver/CrossEntropySolver.h
  Solver/RepeatSolver.h
  Solver/MABMetaSolver.h
  Solver/SurrogateBasedSolver.h
  Solver/SolverCallbacks.h
  Solver/SolverLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SolverLibrary.cpp
)

SET(LBCPP_ML_OPTIMIZER_SOURCES
  Optimizer/SharkMOOptimizers.h
  Optimizer/LBFGSOptimizer.h
  Optimizer/LBFGSOptimizer.cpp
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

SET(LBCPP_ML_SAMPLER_SOURCES
  Sampler/UniformScalarVectorSampler.h
  Sampler/DiagonalGaussianSampler.h
  Sampler/RandomSearchSampler.h
  Sampler/LogLinearActionCodeSearchSampler.h
  Sampler/BinaryMixtureSampler.h
  Sampler/SubsetVectorSampler.h
  Sampler/SamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SamplerLibrary.cpp
)

SET(LBCPP_ML_COMPARATOR_SOURCES
  Comparator/ObjectiveComparator.h
  Comparator/LexicographicComparator.h
  Comparator/DominanceComparator.h
  Comparator/ParetoRankComparator.h
  Comparator/ComparatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ComparatorLibrary.cpp
)

SET(LBCPP_ML_FUNCTION_SOURCES
  Function/BooleanFunctions.h
  Function/IntegerFunctions.h
  Function/DoubleFunctions.h
  Function/EnumerationFunctions.h
  Function/ObjectFunctions.h
  Function/DoubleVectorFunctions.h
  Function/SpecialFunctions.h
  Function/VoteFunctions.h
  Function/Function.cpp
  Function/FunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FunctionLibrary.cpp
)

SET(LBCPP_ML_AGGREGATOR_SOURCES
  Aggregator/MeanDoubleAggregator.h
  Aggregator/MeanDoubleVectorAggregator.h
  Aggregator/AggregatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/AggregatorLibrary.cpp
)

SET(LBCPP_ML_EXPRESSION_SOURCES
  Expression/Expression.cpp
  Expression/ExpressionDomain.cpp
  Expression/ExpressionSampler.cpp
  Expression/PostfixExpression.cpp
  Expression/ExpressionTreeView.h
  Expression/ExpressionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExpressionLibrary.cpp
)

SET(LBCPP_ML_EXPRESSION_SAMPLER_SOURCES
  Expression/FullExpressionSampler.h
  Expression/GrowExpressionSampler.h
  Expression/ScalarExpressionVectorSampler.h
)

SET(LBCPP_ML_EXPRESSION_PERTURBATOR_SOURCES
  Expression/KozaExpressionPerturbator.h
  Expression/InsertExpressionPerturbator.h
  Expression/SwapExpressionPerturbator.h
  Expression/ShrinkExpressionPerturbator.h
  Expression/SubTreeCrossOverExpressionPerturbator.h
)

SET(LBCPP_ML_EXPRESSION_SEARCH_SOURCES
  Expression/ExpressionActionDomainsCache.h
  Expression/PrefixExpressionState.h
  Expression/PostfixExpressionState.h
  Expression/TypedPostfixExpressionState.h
)

SET(LBCPP_ML_EXPRESSION_OBJECTIVE_SOURCES
  Expression/ClassificationObjectives.h
  Expression/RegressionObjectives.h
)

SET(LBCPP_ML_SEARCH_SOURCES
  Search/Search.cpp
  Search/RolloutSearchAlgorithm.h
  Search/LookAheadSearchAlgorithm.h
  Search/StepSearchAlgorithm.h
  Search/SearchLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SearchLibrary.cpp
)

SET(LBCPP_ML_LOADER_SOURCES
  Loader/JdbLoader.h
  Loader/ArffLoader.h
  Loader/LoaderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LoaderLibrary.cpp
)

SET(LBCPP_ML_SPLITTING_CRITERION_SOURCES
  SplittingCriterion/InformationGainSplittingCriterion.h
  SplittingCriterion/StddevReductionSplittingCriterion.h
  SplittingCriterion/SplittingCriterion.cpp
  SplittingCriterion/SplittingCriterionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SplittingCriterionLibrary.cpp
)

SET(LBCPP_ML_LEARNER_SOURCES
  Learner/ExhaustiveConditionLearner.h
  Learner/RandomSplitConditionLearner.h
  Learner/TreeLearner.h
  Learner/EnsembleLearner.h
  Learner/LearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LearnerLibrary.cpp
)

SET(LBCPP_ML_DATA_SOURCES
  Data/FeatureGeneratorCallbacks.hpp
  Data/DoubleVector.cpp
  Data/RandomVariable.cpp
  Data/IndexSet.cpp
  Data/BinaryConfusionMatrix.cpp
  Data/ConfusionMatrixComponent.h
  Data/IterationFunction.hpp
  Data/DataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DataLibrary.cpp
)


SET(LBCPP_ML_TOPLEVEL_SOURCES
  SolutionContainer.cpp
  SolutionContainerComponent.h
  Domain.cpp
  Fitness.cpp
  Problem.cpp
  BanditPool.cpp
)

SET(LBCPP_ML_MAIN_SOURCES
  precompiled.h
  precompiled.cpp
  LBCppMLLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppMLLibrary.cpp
)

SET(LBCPP_ML_SOURCES 
  ${LBCPP_ML_INCLUDES}
  ${LBCPP_ML_SOLVER_SOURCES}
  ${LBCPP_ML_OPTIMIZER_SOURCES}
  ${LBCPP_ML_SAMPLER_SOURCES}
  ${LBCPP_ML_COMPARATOR_SOURCES}
  ${LBCPP_ML_FUNCTION_SOURCES}
  ${LBCPP_ML_AGGREGATOR_SOURCES}
  ${LBCPP_ML_EXPRESSION_SOURCES}
    ${LBCPP_ML_EXPRESSION_SAMPLER_SOURCES}
    ${LBCPP_ML_EXPRESSION_PERTURBATOR_SOURCES}
    ${LBCPP_ML_EXPRESSION_SEARCH_SOURCES}
    ${LBCPP_ML_EXPRESSION_OBJECTIVE_SOURCES}
  ${LBCPP_ML_SEARCH_SOURCES}
  ${LBCPP_ML_LOADER_SOURCES}
  ${LBCPP_ML_SPLITTING_CRITERION_SOURCES}
  ${LBCPP_ML_LEARNER_SOURCES}
  ${LBCPP_ML_DATA_SOURCES}

  ${LBCPP_ML_TOPLEVEL_SOURCES}
  ${LBCPP_ML_MAIN_SOURCES}
)
GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_ML_SOURCES)

SOURCE_GROUP(Include FILES ${LBCPP_ML_INCLUDES})
SOURCE_GROUP(Solver FILES ${LBCPP_ML_SOLVER_SOURCES})
SOURCE_GROUP(Optimizer FILES ${LBCPP_ML_OPTIMIZER_SOURCES})
SOURCE_GROUP(Sampler FILES ${LBCPP_ML_SAMPLER_SOURCES})
SOURCE_GROUP(Comparator FILES ${LBCPP_ML_COMPARATOR_SOURCES})
SOURCE_GROUP(Function FILES ${LBCPP_ML_FUNCTION_SOURCES})
SOURCE_GROUP(Aggregator FILES ${LBCPP_ML_AGGREGATOR_SOURCES})
SOURCE_GROUP(Expression FILES ${LBCPP_ML_EXPRESSION_SOURCES})
SOURCE_SUB_GROUP(Expression Sampler FILES ${LBCPP_ML_EXPRESSION_SAMPLER_SOURCES})
SOURCE_SUB_GROUP(Expression Perturbator FILES ${LBCPP_ML_EXPRESSION_PERTURBATOR_SOURCES})
SOURCE_SUB_GROUP(Expression Search FILES ${LBCPP_ML_EXPRESSION_SEARCH_SOURCES})
SOURCE_SUB_GROUP(Expression Objective FILES ${LBCPP_ML_EXPRESSION_OBJECTIVE_SOURCES})
SOURCE_GROUP(Search FILES ${LBCPP_ML_SEARCH_SOURCES})
SOURCE_GROUP(Loader FILES ${LBCPP_ML_LOADER_SOURCES})
SOURCE_GROUP(SplittingCriterion FILES ${LBCPP_ML_SPLITTING_CRITERION_SOURCES})
SOURCE_GROUP(Learner FILES ${LBCPP_ML_LEARNER_SOURCES})
SOURCE_GROUP(Data FILES ${LBCPP_ML_DATA_SOURCES})

SOURCE_GROUP(Main FILES ${LBCPP_ML_MAIN_SOURCES})

IF (UNIX)
  ADD_LIBRARY(ml SHARED ${LBCPP_ML_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(ml ${LBCPP_ML_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(ml oil)
