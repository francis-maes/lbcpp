INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppMLLibrary.xml LBCppMLLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Comparator/ComparatorLibrary.xml ComparatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/SamplerLibrary.xml SamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Expression/ExpressionLibrary.xml ExpressionLibrary.cpp)

SET(LBCPP_ML_INCLUDES 
  ../include/lbcpp-ml/predeclarations.h
  ../include/lbcpp-ml/Domain.h
  ../include/lbcpp-ml/Fitness.h
  ../include/lbcpp-ml/Solution.h
  ../include/lbcpp-ml/SolutionSet.h
  ../include/lbcpp-ml/SolutionComparator.h
  ../include/lbcpp-ml/Problem.h
  ../include/lbcpp-ml/Sampler.h
  ../include/lbcpp-ml/Optimizer.h
  ../include/lbcpp-ml/Function.h
  ../include/lbcpp-ml/Expression.h
  ../include/lbcpp-ml/ExpressionUniverse.h
  ../include/lbcpp-ml/ExpressionRPN.h
  ../include/lbcpp-ml/ExpressionDomain.h
  ../include/lbcpp-ml/ExpressionSampler.h
)

SET(LBCPP_ML_OPTIMIZER_SOURCES
  Optimizer/Optimizer.cpp
  Optimizer/RandomOptimizer.h
  Optimizer/NRPAOptimizer.h
  Optimizer/CrossEntropyOptimizer.h
  Optimizer/NestedCrossEntropyOptimizer.h
  Optimizer/SharkMOOptimizers.h
  Optimizer/MABMetaOptimizer.h
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

SET(LBCPP_ML_SAMPLER_SOURCES
  Sampler/UniformContinuousSampler.h
  Sampler/DiagonalGaussianSampler.h
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
  Expression/ExpressionRPN.cpp
  Expression/ExpressionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExpressionLibrary.cpp
)

SET(LBCPP_ML_TOPLEVEL_SOURCES
  SolutionSet.cpp
  SolutionSetComponent.h
  Domain.cpp
  Fitness.cpp
  Problem.cpp
)

SET(LBCPP_ML_MAIN_SOURCES
  precompiled.h
  precompiled.cpp
  LBCppMLLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppMLLibrary.cpp
)

SET(LBCPP_ML_SOURCES 
  ${LBCPP_ML_INCLUDES}
  ${LBCPP_ML_OPTIMIZER_SOURCES}
  ${LBCPP_ML_SAMPLER_SOURCES}
  ${LBCPP_ML_COMPARATOR_SOURCES}
  ${LBCPP_ML_FUNCTION_SOURCES}
  ${LBCPP_ML_EXPRESSION_SOURCES}
  ${LBCPP_ML_TOPLEVEL_SOURCES}
  ${LBCPP_ML_MAIN_SOURCES}
)
GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_ML_SOURCES)

SOURCE_GROUP(Include FILES ${LBCPP_ML_INCLUDES})
SOURCE_GROUP(Optimizer FILES ${LBCPP_ML_OPTIMIZER_SOURCES})
SOURCE_GROUP(Sampler FILES ${LBCPP_ML_SAMPLER_SOURCES})
SOURCE_GROUP(Comparator FILES ${LBCPP_ML_COMPARATOR_SOURCES})
SOURCE_GROUP(Function FILES ${LBCPP_ML_FUNCTION_SOURCES})
SOURCE_GROUP(Expression FILES ${LBCPP_ML_EXPRESSION_SOURCES})
SOURCE_GROUP(Main FILES ${LBCPP_ML_MAIN_SOURCES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-ml SHARED ${LBCPP_ML_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-ml ${LBCPP_ML_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-ml lbcpp-core)
