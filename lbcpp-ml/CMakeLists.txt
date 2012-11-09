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
GENERATE_INTROSPECTION_CODE(Expression/ExpressionLibrary.xml ExpressionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Search/SearchLibrary.xml SearchLibrary.cpp)

SET(LBCPP_ML_INCLUDES 
  ../include/lbcpp-ml/predeclarations.h
  ../include/lbcpp-ml/Domain.h
  ../include/lbcpp-ml/Objective.h
  ../include/lbcpp-ml/Fitness.h
  ../include/lbcpp-ml/SolutionContainer.h
  ../include/lbcpp-ml/SolutionComparator.h
  ../include/lbcpp-ml/Problem.h
  ../include/lbcpp-ml/Sampler.h
  ../include/lbcpp-ml/Perturbator.h
  ../include/lbcpp-ml/Solver.h
  ../include/lbcpp-ml/Search.h
  ../include/lbcpp-ml/Function.h
  ../include/lbcpp-ml/Expression.h
  ../include/lbcpp-ml/ExpressionUniverse.h
  ../include/lbcpp-ml/PostfixExpression.h
  ../include/lbcpp-ml/ExpressionDomain.h
  ../include/lbcpp-ml/ExpressionSampler.h
  ../include/lbcpp-ml/BanditPool.h
  ../include/lbcpp-ml/DataTable.h
)


SET(LBCPP_ML_SOLVER_SOURCES
  Solver/Solver.cpp
  Solver/RandomSolver.h
  Solver/NRPASolver.h
  Solver/BeamNRPASolver.h
  Solver/CrossEntropySolver.h
  Solver/RepeatSolver.h
  Solver/MABMetaSolver.h
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
  Sampler/UniformContinuousSampler.h
  Sampler/DiagonalGaussianSampler.h
  Sampler/RandomSearchSampler.h
  Sampler/LogLinearActionCodeSearchSampler.h
  Sampler/BinaryMixtureSampler.h
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

SET(LBCPP_ML_EXPRESSION_SOURCES
  Expression/ExpressionUniverse.cpp
  Expression/Expression.cpp
  Expression/ExpressionDomain.cpp
  Expression/ExpressionSampler.cpp
  Expression/PostfixExpression.cpp
  Expression/ExpressionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExpressionLibrary.cpp
)

SET(LBCPP_ML_EXPRESSION_SAMPLER_SOURCES
  Expression/FullExpressionSampler.h
  Expression/GrowExpressionSampler.h
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

SET(LBCPP_ML_TOPLEVEL_SOURCES
  SolutionContainer.cpp
  SolutionContainerComponent.h
  Domain.cpp
  Fitness.cpp
  Problem.cpp
  BanditPool.cpp
  DataTable.cpp
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
  ${LBCPP_ML_EXPRESSION_SOURCES}
    ${LBCPP_ML_EXPRESSION_SAMPLER_SOURCES}
    ${LBCPP_ML_EXPRESSION_PERTURBATOR_SOURCES}
    ${LBCPP_ML_EXPRESSION_SEARCH_SOURCES}
    ${LBCPP_ML_EXPRESSION_OBJECTIVE_SOURCES}
  ${LBCPP_ML_SEARCH_SOURCES}
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
SOURCE_GROUP(Expression FILES ${LBCPP_ML_EXPRESSION_SOURCES})
SOURCE_SUB_GROUP(Expression Sampler FILES ${LBCPP_ML_EXPRESSION_SAMPLER_SOURCES})
SOURCE_SUB_GROUP(Expression Perturbator FILES ${LBCPP_ML_EXPRESSION_PERTURBATOR_SOURCES})
SOURCE_SUB_GROUP(Expression Search FILES ${LBCPP_ML_EXPRESSION_SEARCH_SOURCES})
SOURCE_SUB_GROUP(Expression Objective FILES ${LBCPP_ML_EXPRESSION_OBJECTIVE_SOURCES})
SOURCE_GROUP(Search FILES ${LBCPP_ML_SEARCH_SOURCES})
SOURCE_GROUP(Main FILES ${LBCPP_ML_MAIN_SOURCES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-ml SHARED ${LBCPP_ML_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-ml ${LBCPP_ML_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-ml lbcpp-core)
