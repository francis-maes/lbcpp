INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/extern/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppMLLibrary.xml LBCppMLLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Comparator/ComparatorLibrary.xml ComparatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/SamplerLibrary.xml SamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(VariableEncoder/VariableEncoderLibrary.xml VariableEncoderLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Solver/SolverLibrary.xml SolverLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Aggregator/AggregatorLibrary.xml AggregatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Expression/ExpressionLibrary.xml ExpressionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Search/SearchLibrary.xml SearchLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Loader/LoaderLibrary.xml LoaderLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SplittingCriterion/SplittingCriterionLibrary.xml SplittingCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(SelectionCriterion/SelectionCriterionLibrary.xml SelectionCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learner/LearnerLibrary.xml LearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(IncrementalLearner/IncrementalLearnerLibrary.xml IncrementalLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)
GENERATE_INTROSPECTION_CODE(GeneticOperator/GeneticOperatorLibrary.xml GeneticOperatorLibrary.cpp)

SET(ML_INCLUDES ../../include/ml)

SET(ML_INCLUDES 
  ${ML_INCLUDES}/predeclarations.h
  ${ML_INCLUDES}/Domain.h
  ${ML_INCLUDES}/Objective.h
  ${ML_INCLUDES}/Fitness.h
  ${ML_INCLUDES}/SolutionContainer.h
  ${ML_INCLUDES}/SolutionComparator.h
  ${ML_INCLUDES}/Problem.h
  ${ML_INCLUDES}/Sampler.h
  ${ML_INCLUDES}/Perturbator.h
  ${ML_INCLUDES}/VariableEncoder.h
  ${ML_INCLUDES}/Solver.h
  ${ML_INCLUDES}/SolverCallback.h
  ${ML_INCLUDES}/Search.h
  ${ML_INCLUDES}/BinarySearchTree.h
  ${ML_INCLUDES}/Function.h
  ${ML_INCLUDES}/Aggregator.h
  ${ML_INCLUDES}/Expression.h
  ${ML_INCLUDES}/PostfixExpression.h
  ${ML_INCLUDES}/ExpressionDomain.h
  ${ML_INCLUDES}/ExpressionSampler.h
  ${ML_INCLUDES}/BanditPool.h
  ${ML_INCLUDES}/SplittingCriterion.h
  ${ML_INCLUDES}/SelectionCriterion.h
  ${ML_INCLUDES}/DoubleVector.h
  ${ML_INCLUDES}/RandomVariable.h  
  ${ML_INCLUDES}/IndexSet.h
  ${ML_INCLUDES}/BinaryKey.h
  ${ML_INCLUDES}/BinaryConfusionMatrix.h
  ${ML_INCLUDES}/IterationFunction.h
  ${ML_INCLUDES}/IncrementalLearner.h
  ${ML_INCLUDES}/DataStream.h
  ${ML_INCLUDES}/GeneticOperator.h
)

SET(ML_VARIABLE_ENCODER_SOURCES
  VariableEncoder/ScalarVectorVariableEncoder.h
  VariableEncoder/VariableEncoderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/VariableEncoderLibrary.cpp
)

SET(ML_SOLVER_SOURCES
  Solver/Solver.cpp
  Solver/RandomSolver.h
  Solver/NRPASolver.h
  Solver/BeamNRPASolver.h
  Solver/CrossEntropySolver.h
  Solver/RepeatSolver.h
  Solver/MABMetaSolver.h
  Solver/SurrogateBasedSolver.h
  Solver/SurrogateBasedSolverInformation.h
  Solver/SurrogateBasedSolverInformationComponent.h
  Solver/SolverCallbacks.h
  Solver/SolverEvaluators.h
  Solver/SolverLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SolverLibrary.cpp
)

SET(ML_OPTIMIZER_SOURCES
  Optimizer/SharkSOOptimizers.h
  Optimizer/SharkMOOptimizers.h
  Optimizer/LBFGSOptimizer.h
  Optimizer/LBFGSOptimizer.cpp
  Optimizer/ParEGOOptimizer.h
  Optimizer/ParEGOOptimizer.cpp
  Optimizer/SMPSOOptimizer.h
  Optimizer/SMPSOOptimizer.cpp
  Optimizer/OMOPSOOptimizer.h
  Optimizer/OMOPSOOptimizer.cpp
  Optimizer/AbYSSOptimizer.h
  Optimizer/AbYSSOptimizer.cpp
  Optimizer/AMOSAOptimizer.h
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

SET(ML_SAMPLER_SOURCES
  Sampler/UniformSampler.h
  Sampler/LatinHypercubeVectorSampler.h
  Sampler/EdgeVectorSampler.h
  Sampler/DiagonalGaussianSampler.h
  Sampler/RandomSearchSampler.h
  Sampler/LogLinearActionCodeSearchSampler.h
  Sampler/BinaryMixtureSampler.h
  Sampler/SubsetVectorSampler.h
  Sampler/SamplerToVectorSampler.h
  Sampler/SamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SamplerLibrary.cpp
)

SET(ML_COMPARATOR_SOURCES
  Comparator/ObjectiveComparator.h
  Comparator/LexicographicComparator.h
  Comparator/DominanceComparator.h
  Comparator/ParetoRankComparator.h
  Comparator/SPEA2Comparator.h
  Comparator/ComparatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ComparatorLibrary.cpp
)

SET(ML_FUNCTION_SOURCES
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

SET(ML_AGGREGATOR_SOURCES
  Aggregator/MeanDoubleAggregator.h
  Aggregator/MeanDoubleVectorAggregator.h
  Aggregator/StatisticsDoubleAggregator.h
  Aggregator/StatisticsDoubleVectorAggregator.h
  Aggregator/Aggregator.cpp
  Aggregator/AggregatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/AggregatorLibrary.cpp
)

SET(ML_EXPRESSION_SOURCES
  Expression/Expression.cpp
  Expression/ExpressionDomain.cpp
  Expression/ExpressionSampler.cpp
  Expression/PostfixExpression.cpp
  Expression/ExpressionTreeView.h
  Expression/ExpressionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExpressionLibrary.cpp
)

SET(ML_EXPRESSION_SAMPLER_SOURCES
  Expression/FullExpressionSampler.h
  Expression/GrowExpressionSampler.h
  Expression/ScalarExpressionVectorSampler.h
)

SET(ML_EXPRESSION_PERTURBATOR_SOURCES
  Expression/KozaExpressionPerturbator.h
  Expression/InsertExpressionPerturbator.h
  Expression/SwapExpressionPerturbator.h
  Expression/ShrinkExpressionPerturbator.h
  Expression/SubTreeCrossOverExpressionPerturbator.h
)

SET(ML_EXPRESSION_SEARCH_SOURCES
  Expression/ExpressionActionDomainsCache.h
  Expression/PrefixExpressionState.h
  Expression/PostfixExpressionState.h
  Expression/TypedPostfixExpressionState.h
)

SET(ML_EXPRESSION_OBJECTIVE_SOURCES
  Expression/ClassificationObjectives.h
  Expression/RegressionObjectives.h
  Expression/MultiRegressionObjectives.h
)

SET(ML_EXPRESSION_TREE_SOURCES
  Expression/ScalarVectorTreeExpression.h
)

SET(ML_SEARCH_SOURCES
  Search/Search.cpp
  Search/RolloutSearchAlgorithm.h
  Search/LookAheadSearchAlgorithm.h
  Search/StepSearchAlgorithm.h
  Search/SearchLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SearchLibrary.cpp
)

SET(ML_LOADER_SOURCES
  Loader/JdbLoader.h
  Loader/ArffLoader.h
  Loader/LoaderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LoaderLibrary.cpp
)

SET(ML_SPLITTING_CRITERION_SOURCES
  SplittingCriterion/InformationGainSplittingCriterion.h
  SplittingCriterion/StddevReductionSplittingCriterion.h
  SplittingCriterion/VectorStddevReductionSplittingCriterion.h
  SplittingCriterion/SplittingCriterion.cpp
  SplittingCriterion/SplittingCriterionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SplittingCriterionLibrary.cpp
)

SET(ML_SELECTION_CRITERION_SOURCES
  SelectionCriterion/GreedySelectionCriterion.h
  SelectionCriterion/OptimisticSelectionCriterion.h
  SelectionCriterion/ProbabilityOfImprovementSelectionCriterion.h
  SelectionCriterion/ExpectedImprovementSelectionCriterion.h
  SelectionCriterion/SelectionCriterionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SelectionCriterionLibrary.cpp
)

SET(ML_LEARNER_SOURCES
  Learner/SharkGaussianProcessLearner.h
  Learner/ExhaustiveConditionLearner.h
  Learner/RandomSplitConditionLearner.h
  Learner/TreeLearner.h
  Learner/EnsembleLearner.h
  Learner/IncrementalLearnerBasedLearner.h
  Learner/LearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LearnerLibrary.cpp
)

SET(ML_INCREMENTAL_LEARNER_SOURCES
  IncrementalLearner/ExtraTreeIncrementalLearner.h
  IncrementalLearner/HoeffdingTreeIncrementalLearner.h
  IncrementalLearner/PerceptronIncrementalLearner.h
  IncrementalLearner/SimpleLinearRegressionIncrementalLearner.h
  IncrementalLearner/IncrementalSplittingCriterion.h
  IncrementalLearner/IncrementalLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/IncrementalLearnerLibrary.cpp
)

SET(ML_DATA_SOURCES
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

SET(ML_GENETIC_OPERATOR_SOURCES
  GeneticOperator/UniformMutation.h
  GeneticOperator/NonUniformMutation.h
  GeneticOperator/PolynomialMutation.h
  GeneticOperator/LocalSearchMutation.h
  GeneticOperator/SBXCrossover.h
  GeneticOperator/GeneticOperatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/GeneticOperatorLibrary.cpp
)

SET(ML_TOPLEVEL_SOURCES
  SolutionContainer.cpp
  SolutionContainerComponent.h
  Domain.cpp
  Fitness.cpp
  Sampler.cpp
  Problem.cpp
  BanditPool.cpp
)

SET(ML_MAIN_SOURCES
  precompiled.h
  precompiled.cpp
  LBCppMLLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppMLLibrary.cpp
)

SET(ML_SOURCES 
  ${ML_INCLUDES}
  ${ML_VARIABLE_ENCODER_SOURCES}
  ${ML_SOLVER_SOURCES}
  ${ML_OPTIMIZER_SOURCES}
  ${ML_SAMPLER_SOURCES}
  ${ML_COMPARATOR_SOURCES}
  ${ML_FUNCTION_SOURCES}
  ${ML_AGGREGATOR_SOURCES}
  ${ML_EXPRESSION_SOURCES}
    ${ML_EXPRESSION_SAMPLER_SOURCES}
    ${ML_EXPRESSION_PERTURBATOR_SOURCES}
    ${ML_EXPRESSION_SEARCH_SOURCES}
    ${ML_EXPRESSION_OBJECTIVE_SOURCES}
    ${ML_EXPRESSION_TREE_SOURCES}
  ${ML_SEARCH_SOURCES}
  ${ML_LOADER_SOURCES}
  ${ML_SPLITTING_CRITERION_SOURCES}
  ${ML_SELECTION_CRITERION_SOURCES}
  ${ML_LEARNER_SOURCES}
  ${ML_INCREMENTAL_LEARNER_SOURCES}
  ${ML_DATA_SOURCES}
  ${ML_GENETIC_OPERATOR_SOURCES}
  ${ML_TOPLEVEL_SOURCES}
  ${ML_MAIN_SOURCES}
)
GENERATE_LBCPP_PRECOMPILED(precompiled ML_SOURCES)

SOURCE_GROUP(Include FILES ${ML_INCLUDES})
SOURCE_GROUP(VariableEncoder FILES ${ML_VARIABLE_ENCODER_SOURCES})
SOURCE_GROUP(Solver FILES ${ML_SOLVER_SOURCES})
SOURCE_GROUP(Optimizer FILES ${ML_OPTIMIZER_SOURCES})
SOURCE_GROUP(Sampler FILES ${ML_SAMPLER_SOURCES})
SOURCE_GROUP(Comparator FILES ${ML_COMPARATOR_SOURCES})
SOURCE_GROUP(Function FILES ${ML_FUNCTION_SOURCES})
SOURCE_GROUP(Aggregator FILES ${ML_AGGREGATOR_SOURCES})
SOURCE_GROUP(Expression FILES ${ML_EXPRESSION_SOURCES})
SOURCE_SUB_GROUP(Expression Sampler FILES ${ML_EXPRESSION_SAMPLER_SOURCES})
SOURCE_SUB_GROUP(Expression Perturbator FILES ${ML_EXPRESSION_PERTURBATOR_SOURCES})
SOURCE_SUB_GROUP(Expression Search FILES ${ML_EXPRESSION_SEARCH_SOURCES})
SOURCE_SUB_GROUP(Expression Objective FILES ${ML_EXPRESSION_OBJECTIVE_SOURCES})
SOURCE_SUB_GROUP(Expression Tree FILES ${ML_EXPRESSION_TREE_SOURCES})
SOURCE_GROUP(Search FILES ${ML_SEARCH_SOURCES})
SOURCE_GROUP(Loader FILES ${ML_LOADER_SOURCES})
SOURCE_GROUP(SplittingCriterion FILES ${ML_SPLITTING_CRITERION_SOURCES})
SOURCE_GROUP(SelectionCriterion FILES ${ML_SELECTION_CRITERION_SOURCES})
SOURCE_GROUP(Learner FILES ${ML_LEARNER_SOURCES})
SOURCE_GROUP(IncrementalLearner FILES ${ML_INCREMENTAL_LEARNER_SOURCES})
SOURCE_GROUP(Data FILES ${ML_DATA_SOURCES})
SOURCE_GROUP(GeneticOperator FILES ${ML_GENETIC_OPERATOR_SOURCES})

SOURCE_GROUP(Main FILES ${ML_MAIN_SOURCES})

IF (UNIX)
  ADD_LIBRARY(ml SHARED ${ML_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(ml ${ML_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(ml oil)
