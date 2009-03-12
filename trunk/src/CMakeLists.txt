INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

SET(CRALGO_LIBRARY_HEADERS
  ../include/cralgo/cralgo.h
  ../include/cralgo/Traits.h
  ../include/cralgo/Random.h
  ../include/cralgo/ContainerTraits.h
  ../include/cralgo/ReferenceCountedObject.h
  ../include/cralgo/ErrorHandler.h
  ../include/cralgo/Object.h
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
  ../include/cralgo/StateFunction.h
  ../include/cralgo/Variable.h
  ../include/cralgo/Choose.h
  ../include/cralgo/Callback.h
  ../include/cralgo/Policy.h
  ../include/cralgo/CRAlgorithmScope.h
  ../include/cralgo/CRAlgorithm.h
  ../include/cralgo/impl/impl.h
)

SET(CRALGO_LIBRARY_IMPL_FEATURE_VISITOR
  ../include/cralgo/impl/FeatureVisitor/FeatureVisitorStatic.hpp
  ../include/cralgo/impl/FeatureVisitor/FeatureVisitorStaticToDynamic.hpp
  ../include/cralgo/impl/FeatureVisitor/FeatureVisitorDynamicToStatic.hpp
  ../include/cralgo/impl/FeatureVisitor/CreateSparseVectorVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/UnaryOperationVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/AssignmentVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/DotProductVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(CRALGO_LIBRARY_IMPL_FUNCTION
  ../include/cralgo/impl/Function/BinaryScalarOperation.hpp
  ../include/cralgo/impl/Function/FunctionStatic.hpp
  ../include/cralgo/impl/Function/FunctionPairTraits.hpp
  ../include/cralgo/impl/Function/FunctionStaticToDynamic.hpp

  ../include/cralgo/impl/Function/ScalarFunctions.hpp
  ../include/cralgo/impl/Function/ScalarVectorFunctions.hpp

  ../include/cralgo/impl/Function/LossFunctions.hpp

  ../include/cralgo/impl/Function/PerceptronLossFunction.hpp
  ../include/cralgo/impl/Function/HingeLossFunction.hpp
  ../include/cralgo/impl/Function/LogBinomialLossFunction.hpp
  ../include/cralgo/impl/Function/ExponentialLossFunction.hpp
  ../include/cralgo/impl/Function/MulticlassLogBinomialLossFunction.hpp

  ../include/cralgo/impl/Function/ExampleRiskFunction.hpp
  ../include/cralgo/impl/Function/EmpiricalRiskFunction.hpp

  ../include/cralgo/impl/Function/BiasArchitecture.hpp
  ../include/cralgo/impl/Function/LinearArchitecture.hpp
  ../include/cralgo/impl/Function/MultiLinearArchitecture.hpp
  ../include/cralgo/impl/Function/TransferArchitecture.hpp
)

SET(CRALGO_LIBRARY_IMPL_POLICY
  ../include/cralgo/impl/Policy/PolicyStatic.hpp
  ../include/cralgo/impl/Policy/PolicyStaticToDynamic.hpp
  ../include/cralgo/impl/Policy/PolicyDynamicToStatic.hpp
  ../include/cralgo/impl/Policy/BasicPolicies.hpp
  ../include/cralgo/impl/Policy/ComputeStatisticsPolicy.hpp
  ../include/cralgo/impl/Policy/QLearningPolicy.hpp
  ../include/cralgo/impl/Policy/MonteCarloControlPolicy.hpp
  ../include/cralgo/impl/Policy/GPOMDPPolicy.hpp
  ../include/cralgo/impl/Policy/ClassificationExampleCreatorPolicy.hpp
  ../include/cralgo/impl/Policy/RankingExampleCreatorPolicy.hpp
)

SET(CRALGO_LIBRARY_IMPL_BRIDGE
  ../include/cralgo/impl/Bridge/StaticInterface.hpp
  ../include/cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp
  ../include/cralgo/impl/Bridge/FeatureGenerator.hpp
  ../include/cralgo/impl/Bridge/DoubleVector.hpp
  ../include/cralgo/impl/Bridge/StateFunction.hpp
  ../include/cralgo/impl/Bridge/Variable.hpp
  ../include/cralgo/impl/Bridge/Choose.hpp
  ../include/cralgo/impl/Bridge/Callback.hpp
  ../include/cralgo/impl/Bridge/CRAlgorithmScope.hpp
  ../include/cralgo/impl/Bridge/CRAlgorithm.hpp
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
  Policy.cpp
)

SET(CRALGO_LIBRARY
  ${CRALGO_LIBRARY_HEADERS}
  ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR}
  ${CRALGO_LIBRARY_IMPL_FUNCTION}
  ${CRALGO_LIBRARY_IMPL_POLICY}
  ${CRALGO_LIBRARY_IMPL_BRIDGE}
  ${CRALGO_LIBRARY_SOURCES}
)

SOURCE_GROUP(includes FILES ${CRALGO_LIBRARY_HEADERS})
SOURCE_GROUP(impl/FeatureVisitor FILES ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR})
SOURCE_GROUP(impl/Function FILES ${CRALGO_LIBRARY_IMPL_FUNCTION})
SOURCE_GROUP(impl/Policy FILES ${CRALGO_LIBRARY_IMPL_POLICY})
SOURCE_GROUP(impl/Bridge FILES ${CRALGO_LIBRARY_IMPL_BRIDGE})
SOURCE_GROUP(sources FILES ${CRALGO_LIBRARY_SOURCES})

ADD_LIBRARY(cralgo ${CRALGO_LIBRARY})
