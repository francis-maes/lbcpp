INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

SET(LCPP_CORE_IMPL ../include/lcpp/impl)

SET(LCPP_CORE_HEADERS
  ../include/lcpp/lcpp.h
  ../include/lcpp/Traits.h
  ../include/lcpp/Random.h
  ../include/lcpp/ContainerTraits.h
  ../include/lcpp/ReferenceCountedObject.h
  ../include/lcpp/Utilities.h
  ../include/lcpp/Object.h
  ../include/lcpp/ObjectPredeclarations.h
  ../include/lcpp/RandomVariable.h
  ../include/lcpp/FeatureDictionary.h
  ../include/lcpp/FeatureGenerator.h
  ../include/lcpp/EditableFeatureGenerator.h
  ../include/lcpp/SparseVector.h
  ../include/lcpp/DenseVector.h
  ../include/lcpp/LearningExample.h
  ../include/lcpp/LearningMachine.h
  ../include/lcpp/ContinuousFunction.h
  ../include/lcpp/IterationFunction.h
  ../include/lcpp/Optimizer.h
  ../include/lcpp/GradientBasedLearner.h
  ../include/lcpp/GradientBasedLearningMachine.h
  ../include/lcpp/ChooseFunction.h
  ../include/lcpp/Variable.h
  ../include/lcpp/Choose.h
  ../include/lcpp/Callback.h
  ../include/lcpp/Policy.h
  ../include/lcpp/CRAlgorithmScope.h
  ../include/lcpp/CRAlgorithm.h
)

SET(LCPP_CORE_IMPL_HEADERS
  ${LCPP_CORE_IMPL}/Macros.hpp
  ${LCPP_CORE_IMPL}/Object.hpp
  ${LCPP_CORE_IMPL}/StaticToDynamic.hpp
  ${LCPP_CORE_IMPL}/DynamicToStatic.hpp
  ${LCPP_CORE_IMPL}/impl.h
)

SET(LCPP_CORE_IMPL_FEATURE_VISITOR
  ${LCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorStatic.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorStaticToDynamic.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorDynamicToStatic.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/CreateSparseVectorVisitor.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/UnaryOperationVisitor.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/AssignmentVisitor.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/DotProductVisitor.hpp
  ${LCPP_CORE_IMPL}/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(LCPP_CORE_IMPL_CONTINUOUS_FUNCTION
  ${LCPP_CORE_IMPL}/ContinuousFunction/BinaryScalarOperation.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/FunctionStatic.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/FunctionPairTraits.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/FunctionStaticToDynamic.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/FunctionDynamicToStatic.hpp

  ${LCPP_CORE_IMPL}/ContinuousFunction/ScalarFunctions.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/ScalarVectorFunctions.hpp

  ${LCPP_CORE_IMPL}/ContinuousFunction/LossFunctions.hpp

  ${LCPP_CORE_IMPL}/ContinuousFunction/AbsoluteLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/SquareLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/PerceptronLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/HingeLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/LogBinomialLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/ExponentialLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/MulticlassLogBinomialLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/AllPairsLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/MostViolatedPairLossFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/BestAgainstAllLossFunction.hpp

  ${LCPP_CORE_IMPL}/ContinuousFunction/ExampleRiskFunction.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/EmpiricalRiskFunction.hpp

  ${LCPP_CORE_IMPL}/ContinuousFunction/BiasArchitecture.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/LinearArchitecture.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/MultiLinearArchitecture.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/TransferArchitecture.hpp
  ${LCPP_CORE_IMPL}/ContinuousFunction/ScalarToVectorArchitecture.hpp
)

SET(LCPP_CORE_IMPL_CHOOSE_FUNCTION
  ${LCPP_CORE_IMPL}/ChooseFunction/ChooseFunctionStatic.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/ChooseFunctionStaticToDynamic.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/ChooseFunctionDynamicToStatic.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/CompositeValueFunctions.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/ClassifierValueFunctions.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/RegressorValueFunctions.hpp
  ${LCPP_CORE_IMPL}/ChooseFunction/RankerValueFunctions.hpp
)

SET(LCPP_CORE_IMPL_POLICY
  ${LCPP_CORE_IMPL}/Policy/PolicyStatic.hpp
  ${LCPP_CORE_IMPL}/Policy/PolicyStaticToDynamic.hpp
  ${LCPP_CORE_IMPL}/Policy/PolicyDynamicToStatic.hpp
  ${LCPP_CORE_IMPL}/Policy/BasicPolicies.hpp
  ${LCPP_CORE_IMPL}/Policy/ComputeStatisticsPolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/VerbosePolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/QLearningPolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/MonteCarloControlPolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/GPOMDPPolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/ClassificationExampleCreatorPolicy.hpp
  ${LCPP_CORE_IMPL}/Policy/RankingExampleCreatorPolicy.hpp
)

SET(LCPP_CORE_IMPL_BRIDGE
  ${LCPP_CORE_IMPL}/Bridge/StaticInterface.hpp
  ${LCPP_CORE_IMPL}/Bridge/FeatureGeneratorDefaultImplementations.hpp
  ${LCPP_CORE_IMPL}/Bridge/FeatureGenerator.hpp
  ${LCPP_CORE_IMPL}/Bridge/DoubleVector.hpp
  ${LCPP_CORE_IMPL}/Bridge/Variable.hpp
  ${LCPP_CORE_IMPL}/Bridge/Choose.hpp
  ${LCPP_CORE_IMPL}/Bridge/Callback.hpp
  ${LCPP_CORE_IMPL}/Bridge/CRAlgorithmScope.hpp
  ${LCPP_CORE_IMPL}/Bridge/CRAlgorithm.hpp
)

SET(LCPP_CORE_SOURCES
  classes.cpp
  Object.cpp
  Utilities.cpp
  FeatureDictionary.cpp
  FeatureGenerator.cpp
  SparseVector.cpp
  DenseVector.cpp
  DoubleVectorHelpers.hpp
  ContinuousFunction.cpp
  Optimizer.cpp
  LearningExample.cpp
  LearningMachine.cpp
  GradientBasedLearner.cpp
  GradientBasedLearningMachine.cpp
  Choose.cpp
  Policy.cpp
)

SET(LCPP_CORE_FEATURE_GENERATOR_SOURCES
  FeatureGenerator/EmptyFeatureGenerator.h
  FeatureGenerator/UnitFeatureGenerator.h
  FeatureGenerator/SubFeatureGenerator.h
  FeatureGenerator/WeightedFeatureGenerator.h
  FeatureGenerator/LinearCombinationFeatureGenerator.h
  FeatureGenerator/ExplicitLinearCombinationFeatureGenerator.h
)

SET(LCPP_CORE_OPTIMIZER_SOURCES
  Optimizer/RPropOptimizer.h
  Optimizer/GradientDescentOptimizer.h
  Optimizer/BackTrackingLineSearch.h
  Optimizer/QuasiNewtonMemory.h
  Optimizer/LBFGSOptimizer.h
)

SET(LCPP_CORE
  ${LCPP_CORE_HEADERS}
  ${LCPP_CORE_IMPL_HEADERS}
  ${LCPP_CORE_IMPL_FEATURE_VISITOR}
  ${LCPP_CORE_IMPL_CONTINUOUS_FUNCTION}
  ${LCPP_CORE_IMPL_OPTIMIZER}
  ${LCPP_CORE_IMPL_CHOOSE_FUNCTION}
  ${LCPP_CORE_IMPL_POLICY}
  ${LCPP_CORE_IMPL_BRIDGE}
  ${LCPP_CORE_SOURCES}
  ${LCPP_CORE_FEATURE_GENERATOR_SOURCES}
  ${LCPP_CORE_OPTIMIZER_SOURCES}
)

SOURCE_GROUP(includes FILES ${LCPP_CORE_HEADERS})
SOURCE_GROUP(impl FILES ${LCPP_CORE_IMPL_HEADERS})
SOURCE_GROUP(impl/FeatureVisitor FILES ${LCPP_CORE_IMPL_FEATURE_VISITOR})
SOURCE_GROUP(impl/ContinuousFunction FILES ${LCPP_CORE_IMPL_CONTINUOUS_FUNCTION})
SOURCE_GROUP(impl/Optimizer FILES ${LCPP_CORE_IMPL_OPTIMIZER})
SOURCE_GROUP(impl/ChooseFunction FILES ${LCPP_CORE_IMPL_CHOOSE_FUNCTION})
SOURCE_GROUP(impl/Policy FILES ${LCPP_CORE_IMPL_POLICY})
SOURCE_GROUP(impl/Bridge FILES ${LCPP_CORE_IMPL_BRIDGE})
SOURCE_GROUP(sources FILES ${LCPP_CORE_SOURCES})
SOURCE_GROUP(FeatureGenerator FILES ${LCPP_CORE_FEATURE_GENERATOR_SOURCES})
SOURCE_GROUP(Optimizer FILES ${LCPP_CORE_OPTIMIZER_SOURCES})

ADD_LIBRARY(lcpp ${LCPP_CORE})
