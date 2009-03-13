INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

SET(CRALGO_IMPL ../include/cralgo/impl)

SET(CRALGO_LIBRARY_HEADERS
  ../include/cralgo/cralgo.h
  ../include/cralgo/Traits.h
  ../include/cralgo/Random.h
  ../include/cralgo/ContainerTraits.h
  ../include/cralgo/ReferenceCountedObject.h
  ../include/cralgo/Object.h
  ../include/cralgo/ObjectPredeclarations.h
  ../include/cralgo/RandomVariable.h
  ../include/cralgo/FeatureDictionary.h
  ../include/cralgo/FeatureGenerator.h
  ../include/cralgo/DoubleVector.h
  ../include/cralgo/SparseVector.h
  ../include/cralgo/DenseVector.h
  ../include/cralgo/LazyVector.h
  ../include/cralgo/LearningExample.h
  ../include/cralgo/LearningMachine.h
  ../include/cralgo/GradientBasedLearner.h
  ../include/cralgo/GradientBasedLearningMachine.h
  ../include/cralgo/ContinuousFunction.h
  ../include/cralgo/ChooseFunction.h
  ../include/cralgo/Variable.h
  ../include/cralgo/Choose.h
  ../include/cralgo/Callback.h
  ../include/cralgo/Policy.h
  ../include/cralgo/CRAlgorithmScope.h
  ../include/cralgo/CRAlgorithm.h
)

SET(CRALGO_LIBRARY_IMPL_HEADERS
  ${CRALGO_IMPL}/Macros.hpp
  ${CRALGO_IMPL}/Object.hpp
  ${CRALGO_IMPL}/StaticToDynamic.hpp
  ${CRALGO_IMPL}/DynamicToStatic.hpp
  ${CRALGO_IMPL}/impl.h
)

SET(CRALGO_LIBRARY_IMPL_FEATURE_VISITOR
  ${CRALGO_IMPL}/FeatureVisitor/FeatureVisitorStatic.hpp
  ${CRALGO_IMPL}/FeatureVisitor/FeatureVisitorStaticToDynamic.hpp
  ${CRALGO_IMPL}/FeatureVisitor/FeatureVisitorDynamicToStatic.hpp
  ${CRALGO_IMPL}/FeatureVisitor/CreateSparseVectorVisitor.hpp
  ${CRALGO_IMPL}/FeatureVisitor/UnaryOperationVisitor.hpp
  ${CRALGO_IMPL}/FeatureVisitor/AssignmentVisitor.hpp
  ${CRALGO_IMPL}/FeatureVisitor/DotProductVisitor.hpp
  ${CRALGO_IMPL}/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(CRALGO_LIBRARY_IMPL_CONTINUOUS_FUNCTION
  ${CRALGO_IMPL}/ContinuousFunction/BinaryScalarOperation.hpp
  ${CRALGO_IMPL}/ContinuousFunction/FunctionStatic.hpp
  ${CRALGO_IMPL}/ContinuousFunction/FunctionPairTraits.hpp
  ${CRALGO_IMPL}/ContinuousFunction/FunctionStaticToDynamic.hpp

  ${CRALGO_IMPL}/ContinuousFunction/ScalarFunctions.hpp
  ${CRALGO_IMPL}/ContinuousFunction/ScalarVectorFunctions.hpp

  ${CRALGO_IMPL}/ContinuousFunction/LossFunctions.hpp

  ${CRALGO_IMPL}/ContinuousFunction/PerceptronLossFunction.hpp
  ${CRALGO_IMPL}/ContinuousFunction/HingeLossFunction.hpp
  ${CRALGO_IMPL}/ContinuousFunction/LogBinomialLossFunction.hpp
  ${CRALGO_IMPL}/ContinuousFunction/ExponentialLossFunction.hpp
  ${CRALGO_IMPL}/ContinuousFunction/MulticlassLogBinomialLossFunction.hpp

  ${CRALGO_IMPL}/ContinuousFunction/ExampleRiskFunction.hpp
  ${CRALGO_IMPL}/ContinuousFunction/EmpiricalRiskFunction.hpp

  ${CRALGO_IMPL}/ContinuousFunction/BiasArchitecture.hpp
  ${CRALGO_IMPL}/ContinuousFunction/LinearArchitecture.hpp
  ${CRALGO_IMPL}/ContinuousFunction/MultiLinearArchitecture.hpp
  ${CRALGO_IMPL}/ContinuousFunction/TransferArchitecture.hpp
)

SET(CRALGO_LIBRARY_IMPL_CHOOSE_FUNCTION
  ${CRALGO_IMPL}/ChooseFunction/ChooseFunctionStatic.hpp
  ${CRALGO_IMPL}/ChooseFunction/ChooseFunctionStaticToDynamic.hpp
  ${CRALGO_IMPL}/ChooseFunction/ChooseFunctionDynamicToStatic.hpp
  ${CRALGO_IMPL}/ChooseFunction/CompositeValueFunctions.hpp
  ${CRALGO_IMPL}/ChooseFunction/ClassifierValueFunctions.hpp
  ${CRALGO_IMPL}/ChooseFunction/RegressorValueFunctions.hpp
  ${CRALGO_IMPL}/ChooseFunction/RankerValueFunctions.hpp
)

SET(CRALGO_LIBRARY_IMPL_POLICY
  ${CRALGO_IMPL}/Policy/PolicyStatic.hpp
  ${CRALGO_IMPL}/Policy/PolicyStaticToDynamic.hpp
  ${CRALGO_IMPL}/Policy/PolicyDynamicToStatic.hpp
  ${CRALGO_IMPL}/Policy/BasicPolicies.hpp
  ${CRALGO_IMPL}/Policy/ComputeStatisticsPolicy.hpp
  ${CRALGO_IMPL}/Policy/ConsoleLoggerPolicy.hpp
  ${CRALGO_IMPL}/Policy/QLearningPolicy.hpp
  ${CRALGO_IMPL}/Policy/MonteCarloControlPolicy.hpp
  ${CRALGO_IMPL}/Policy/GPOMDPPolicy.hpp
  ${CRALGO_IMPL}/Policy/ClassificationExampleCreatorPolicy.hpp
  ${CRALGO_IMPL}/Policy/RankingExampleCreatorPolicy.hpp
)

SET(CRALGO_LIBRARY_IMPL_BRIDGE
  ${CRALGO_IMPL}/Bridge/StaticInterface.hpp
  ${CRALGO_IMPL}/Bridge/FeatureGeneratorDefaultImplementations.hpp
  ${CRALGO_IMPL}/Bridge/FeatureGenerator.hpp
  ${CRALGO_IMPL}/Bridge/DoubleVector.hpp
  ${CRALGO_IMPL}/Bridge/Variable.hpp
  ${CRALGO_IMPL}/Bridge/Choose.hpp
  ${CRALGO_IMPL}/Bridge/Callback.hpp
  ${CRALGO_IMPL}/Bridge/CRAlgorithmScope.hpp
  ${CRALGO_IMPL}/Bridge/CRAlgorithm.hpp
)

SET(CRALGO_LIBRARY_SOURCES
  classes.cpp
  Object.cpp
  ErrorHandler.cpp
  FeatureDictionary.cpp
  SparseVector.cpp
  DenseVector.cpp
  LazyVector.cpp
  DoubleVectorHelpers.hpp
  LearningExample.cpp
  LearningMachine.cpp
  GradientBasedLearner.cpp
  Choose.cpp
  Policy.cpp
)

SET(CRALGO_LIBRARY
  ${CRALGO_LIBRARY_HEADERS}
  ${CRALGO_LIBRARY_IMPL_HEADERS}
  ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR}
  ${CRALGO_LIBRARY_IMPL_CONTINUOUS_FUNCTION}
  ${CRALGO_LIBRARY_IMPL_CHOOSE_FUNCTION}
  ${CRALGO_LIBRARY_IMPL_POLICY}
  ${CRALGO_LIBRARY_IMPL_BRIDGE}
  ${CRALGO_LIBRARY_SOURCES}
)

SOURCE_GROUP(includes FILES ${CRALGO_LIBRARY_HEADERS})
SOURCE_GROUP(impl FILES ${CRALGO_LIBRARY_IMPL_HEADERS})
SOURCE_GROUP(impl/FeatureVisitor FILES ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR})
SOURCE_GROUP(impl/ContinuousFunction FILES ${CRALGO_LIBRARY_IMPL_CONTINUOUS_FUNCTION})
SOURCE_GROUP(impl/ChooseFunction FILES ${CRALGO_LIBRARY_IMPL_CHOOSE_FUNCTION})
SOURCE_GROUP(impl/Policy FILES ${CRALGO_LIBRARY_IMPL_POLICY})
SOURCE_GROUP(impl/Bridge FILES ${CRALGO_LIBRARY_IMPL_BRIDGE})
SOURCE_GROUP(sources FILES ${CRALGO_LIBRARY_SOURCES})

ADD_LIBRARY(cralgo ${CRALGO_LIBRARY})
