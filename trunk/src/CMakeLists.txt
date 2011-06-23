INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppLibrary.xml LBCppLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Core/Type/CoreTypeLibrary.xml CoreTypeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/Function/CoreFunctionLibrary.xml CoreFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/CoreLibrary.xml CoreLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Data/Stream/StreamLibrary.xml StreamLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerLibrary.xml ContainerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/Matrix/MatrixLibrary.xml MatrixLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

IF(LBCPP_NETWORKING)
  GENERATE_INTROSPECTION_CODE(Network/NetworkLibrary.xml NetworkLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(Network/Grid/GridNetworkLibrary.xml GridNetworkLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(Network/Manager/ManagerNetworkLibrary.xml ManagerNetworkLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(Network/WorkUnit/NetworkWorkUnitLibrary.xml NetworkWorkUnitLibrary.cpp)
ENDIF(LBCPP_NETWORKING)

GENERATE_INTROSPECTION_CODE(Distribution/DistributionLibrary.xml DistributionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Distribution/Function/DistributionFunctionLibrary.xml DistributionFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Distribution/Builder/DistributionBuilderLibrary.xml DistributionBuilderLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Sampler/SamplerLibrary.xml SamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/Continuous/ContinuousSamplerLibrary.xml ContinuousSamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/Discrete/DiscreteSamplerLibrary.xml DiscreteSamplerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Sampler/Composite/CompositeSamplerLibrary.xml CompositeSamplerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionLibrary.xml ScalarFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarVectorFunction/ScalarVectorFunctionLibrary.xml ScalarVectorFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/IterationFunction/IterationFunctionLibrary.xml IterationFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/StoppingCriterion/StoppingCriterionLibrary.xml StoppingCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Predicate/PredicateLibrary.xml PredicateLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Evaluator/EvaluatorLibrary.xml EvaluatorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(FeatureGenerator/FeatureGeneratorLibrary.xml FeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Generic/GenericFeatureGeneratorLibrary.xml GenericFeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Atomic/AtomicFeatureGeneratorLibrary.xml AtomicFeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Composite/CompositeFeatureGeneratorLibrary.xml CompositeFeatureGeneratorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Learning/LearningLibrary.xml LearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/OnlineLearner/OnlineLearnerLibrary.xml OnlineLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/BatchLearner/BatchLearnerLibrary.xml BatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/Numerical/NumericalLearningLibrary.xml NumericalLearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/Numerical/LibSVM/LibSVMLearningLibrary.xml LibSVMLearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/DecisionTree/DecisionTreeLibrary.xml DecisionTreeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/DecisionTree/Data/DecisionTreeDataLibrary.xml DecisionTreeDataLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/LossFunction/LearningLossFunctionLibrary.xml LearningLossFunctionLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/Context/OptimizerContextLibrary.xml OptimizerContextLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/ObjectiveFunction/ObjectiveFunctionLibrary.xml ObjectiveFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/Optimizer/OptimizerOptimizerLibrary.xml OptimizerOptimizerLibrary.cpp)
IF(LBCPP_NETWORKING)
  GENERATE_INTROSPECTION_CODE(Optimizer/Context/Network/OptimizerContextNetworkLibrary.xml OptimizerContextNetworkLibrary.cpp)
ENDIF(LBCPP_NETWORKING)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

#### Core ####

SET(LBCPP_CORE_SOURCES
  ../include/lbcpp/Core/Utilities.h  
  Core/Utilities.cpp
  ../include/lbcpp/Core/ReferenceCountedObject.h
  ../include/lbcpp/Core/predeclarations.h
  ../include/lbcpp/Core/Object.h
  Core/Object.cpp
  ../include/lbcpp/Core/TypeManager.h
  Core/TypeManager.cpp
  ../include/lbcpp/Core/Type.h
  Core/Type.cpp
  ../include/lbcpp/Core/Enumeration.h
  Core/Enumeration.cpp
  ../include/lbcpp/Core/Class.h
  Core/Class.cpp
  ../include/lbcpp/Core/TemplateType.h
  Core/TemplateType.cpp
  ../include/lbcpp/Core/Library.h
  ../include/lbcpp/Core/Variable.h
  Core/Variable.cpp
  ../include/lbcpp/Core/Pair.h
  ../include/lbcpp/Core/Container.h
  Core/Container.cpp
  ../include/lbcpp/Core/Vector.h
  Core/Vector.cpp
  ../include/lbcpp/Core/Map.h
  Core/Map.cpp
  ../include/lbcpp/Core/OrderedMap.h
  Core/OrderedMap.cpp
  ../include/lbcpp/Core/DynamicObject.h
  Core/DynamicObject.cpp
  ../include/lbcpp/Core/Function.h
  Core/Function.cpp  
  ../include/lbcpp/Core/Frame.h
  Core/Frame.cpp
  ../include/lbcpp/Core/CompositeFunction.h
  Core/CompositeFunction.cpp  
  ../include/lbcpp/Core/XmlSerialisation.h
  Core/XmlSerialisation.cpp
  Core/CoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLibrary.cpp
)

SET(LBCPP_CORE_IMPL_HEADERS
  ../include/lbcpp/Core/impl/VariableValue.hpp
  ../include/lbcpp/Core/impl/Variable.hpp
)

SET(LBCPP_CORE_TYPE_SOURCES
  Core/Type/TopLevelType.h
  Core/Type/NilType.h
  Core/Type/BuiltinType.h
  Core/Type/BooleanType.h
  Core/Type/IntegerType.h
  Core/Type/DoubleType.h
  Core/Type/StringType.h
  Core/Type/FileType.h
  Core/Type/CoreTypeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreTypeLibrary.cpp
)

SET(LBCPP_CORE_OBJECT_SOURCES
  Core/Object/SparseVectorHelper.h
  Core/Object/SparseGenericObject.h
  Core/Object/SparseDoubleObject.h
  Core/Object/DenseGenericObject.h
  Core/Object/DenseDoubleObject.h
  Core/Object/DenseObjectObject.h
)

SET(LBCPP_CORE_FUNCTION_SOURCES
  Core/Function/GetVariableFunction.h
  Core/Function/GetElementFunction.h
  Core/Function/CreateObjectFunction.h
  Core/Function/CreateVectorFunction.h
  Core/Function/CreateSymmetricMatrixFunction.h
  Core/Function/LoadFromFileFunction.h
  Core/Function/MapContainerFunction.h
  Core/Function/MapSymmetricMatrixFunction.h
  Core/Function/ComposeFunction.h
  Core/Function/ConvertToDoubleFunction.h
  Core/Function/SignedScalarToProbabilityFunction.h
  Core/Function/ConcatenateContainerFunction.h
  Core/Function/CoreFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreFunctionLibrary.cpp
)


#### Data ####

SET(LBCPP_DATA_SOURCES
  ../include/lbcpp/Data/predeclarations.h
  ../include/lbcpp/Data/Stream.h
  Data/Stream.cpp
  ../include/lbcpp/Data/DoubleVector.h
  Data/DoubleVector.cpp
  ../include/lbcpp/Data/Matrix.h
  ../include/lbcpp/Data/SymmetricMatrix.h
  ../include/lbcpp/Data/RandomGenerator.h
  Data/RandomGenerator.cpp  
  ../include/lbcpp/Data/RandomVariable.h  
  Data/RandomVariable.cpp
  ../include/lbcpp/Data/Consumer.h
  Data/Consumer.cpp
  ../include/lbcpp/Data/Cache.h
  Data/Cache.cpp
  Data/DataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DataLibrary.cpp
)

SET(LBCPP_DATA_MATRIX_SOURCES
  Data/Matrix/Matrix.cpp
  Data/Matrix/UpperLowerSquareMatrix.h
  Data/Matrix/SegmentMatrixFunction.h
  Data/Matrix/MatrixLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MatrixLibrary.cpp
)

SET(LBCPP_DATA_STREAM_SOURCES
  Data/Stream/ApplyFunctionStream.h
  Data/Stream/DirectoryFileStream.h
  Data/Stream/DirectoryPairFileStream.h
  Data/Stream/LibSVMDataParser.h
  Data/Stream/LibSVMDataParser.cpp
  Data/Stream/ARFFDataParser.h
  Data/Stream/ARFFDataParser.cpp
  Data/Stream/SaveToFileConsumer.h
  Data/Stream/StreamLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/StreamLibrary.cpp
)

SET(LBCPP_DATA_CONTAINER_SOURCES
  Data/Container/ApplyFunctionContainer.h
  Data/Container/SubsetContainer.h
  Data/Container/DuplicatedContainer.h
  Data/Container/RangeContainer.h
  Data/Container/ExcludeRangeContainer.h
  Data/Container/AccumulateContainerFunction.h
  Data/Container/AccumulatorLocalMeanFunction.h
  Data/Container/AccumulatorGlobalMeanFunction.h
  Data/Container/AccumulatorWindowMeanFunction.h
  Data/Container/SegmentContainerFunction.h
  Data/Container/ContainerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerLibrary.cpp
)

#### Execution ####

SET(LBCPP_EXECUTION_SOURCES
  ../include/lbcpp/Execution/predeclarations.h
  ../include/lbcpp/Execution/WorkUnit.h
  Execution/WorkUnit.cpp
  ../include/lbcpp/Execution/TestUnit.h
  ../include/lbcpp/Execution/ExecutionStack.h
  ../include/lbcpp/Execution/ExecutionCallback.h
  Execution/ExecutionCallback.cpp
  ../include/lbcpp/Execution/ExecutionContext.h
  Execution/ExecutionContext.cpp
  ../include/lbcpp/Execution/ExecutionTrace.h
  Execution/ExecutionTrace.cpp
  ../include/lbcpp/Execution/Notification.h
  Execution/Notification.cpp
  Execution/ExecutionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionLibrary.cpp
)

SET(LBCPP_EXECUTION_CALLBACK_SOURCES
  Execution/Callback/ExecutionNotifications.h
  Execution/Callback/ConsoleExecutionCallback.h
  Execution/Callback/MakeTraceExecutionCallback.h
  Execution/Callback/MakeAndAutoSaveTraceExecutionCallback.h
  Execution/Callback/ExecutionCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackLibrary.cpp
)
IF(LBCPP_USER_INTERFACE)
  SET(LBCPP_EXECUTION_CALLBACK_SOURCES ${LBCPP_EXECUTION_CALLBACK_SOURCES} Execution/Callback/UserInterfaceExecutionCallback.h)
ENDIF(LBCPP_USER_INTERFACE)


SET(LBCPP_EXECUTION_CONTEXT_SOURCES
  Execution/Context/SingleThreadedExecutionContext.h
  Execution/Context/SubExecutionContext.h
  Execution/Context/MultiThreadedExecutionContext.h
  Execution/Context/ExecutionContextLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionContextLibrary.cpp
)

### Network ###
IF(LBCPP_NETWORKING)
  SET(LBCPP_NETWORK_SOURCES
    ../include/lbcpp/Network/NetworkClient.h
    Network/NetworkClient.cpp
    ../include/lbcpp/Network/NetworkServer.h
    Network/NetworkServer.cpp
    ../include/lbcpp/Network/NetworkCallback.h
    ../include/lbcpp/Network/NetworkInterface.h
    ../include/lbcpp/Network/NetworkNotification.h
    ../include/lbcpp/Network/WorkUnitNetworkRequest.h
    Network/WorkUnitNetworkRequest.cpp
    Network/NetworkLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/NetworkLibrary.cpp
  )
  
  SET(LBCPP_NETWORK_GRID_SOURCES
    Network/Grid/GridNetworkInterface.h
    Network/Grid/GridNetworkInterface.cpp
    Network/Grid/GridNetworkNotification.h
    Network/Grid/GridNetworkLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/GridNetworkLibrary.cpp
  )
  
  SET(LBCPP_NETWORK_MANAGER_SOURCES
    Network/Manager/ManagerNetworkInterface.h
    Network/Manager/ManagerNetworkInterface.cpp
    Network/Manager/ManagerNetworkNotification.h
    Network/Manager/NetworkProjectFileManager.h
    Network/Manager/ManagerNetworkLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/ManagerNetworkLibrary.cpp
  )
  
  SET(LBCPP_NETWORK_WORK_UNIT_SOURCES
    Network/WorkUnit/NetworkWorkUnit.h
    Network/WorkUnit/NetworkWorkUnit.cpp
    Network/WorkUnit/NetworkWorkUnitLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/NetworkWorkUnitLibrary.cpp
  )

ENDIF(LBCPP_NETWORKING)

#### Sampler / Distribution ####

SET(LBCPP_SAMPLER_SOURCES
  ../include/lbcpp/Sampler/Sampler.h
  Sampler/Sampler.cpp
  Sampler/SamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SamplerLibrary.cpp
)

SET(LBCPP_SAMPLER_DISCRETE_SOURCES
  Sampler/Discrete/BernoulliSampler.h
  Sampler/Discrete/EnumerationSampler.h
  Sampler/Discrete/DiscretizeSampler.h
  Sampler/Discrete/MaximumEntropySampler.h
  Sampler/Discrete/DiscreteSamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DiscreteSamplerLibrary.cpp
)

SET(LBCPP_SAMPLER_CONTINUOUS_SOURCES
  Sampler/Continuous/GaussianSampler.h
  Sampler/Continuous/MultiVariateGaussianSampler.h
  Sampler/Continuous/UniformScalarSampler.h
  Sampler/Continuous/ConditionalGaussianSampler.h
  Sampler/Continuous/ContinuousSamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContinuousSamplerLibrary.cpp
)

SET(LBCPP_SAMPLER_COMPOSITE_SOURCES
  Sampler/Composite/ObjectCompositeSampler.h
  Sampler/Composite/MixtureSampler.h
  Sampler/Composite/RejectionSampler.h
  Sampler/Composite/IndependentDoubleVectorSampler.h
  Sampler/Composite/IndependentDoubleMatrixSampler.h
  Sampler/Composite/ZeroOrScalarContinuousSampler.h
  Sampler/Composite/CompositeSamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CompositeSamplerLibrary.cpp
)

SET(LBCPP_DISTRIBUTION_SOURCES
  ../include/lbcpp/Distribution/predeclarations.h
  ../include/lbcpp/Distribution/Distribution.h
  ../include/lbcpp/Distribution/DiscreteDistribution.h
  Distribution/DiscreteDistribution.cpp
  ../include/lbcpp/Distribution/ContinuousDistribution.h
  Distribution/ContinuousDistribution.cpp
  ../include/lbcpp/Distribution/MultiVariateDistribution.h
  Distribution/MultiVariateDistribution.cpp
  Distribution/DistributionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DistributionLibrary.cpp
)

SET(LBCPP_DISTRIBUTION_FUNCTION_SOURCES
  Distribution/Function/DistributionEntropyFunction.h
  Distribution/Function/SampleDistributionFunction.h
  Distribution/Function/DistributionFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DistributionFunctionLibrary.cpp
)

SET(LBCPP_DISTRIBUTION_BUILDER_SOURCES
  ../include/lbcpp/Distribution/DistributionBuilder.h
  Distribution/Builder/EnumerationDistributionBuilder.h
  Distribution/Builder/GaussianDistributionBuilder.h
  Distribution/Builder/GaussianDistributionBuilder.cpp
  Distribution/Builder/BernoulliDistributionBuilder.h
  Distribution/Builder/IndependentMultiVariateDistributionBuilder.h
  Distribution/Builder/IndependentDoubleVectorDistributionBuilder.h
  Distribution/Builder/DistributionBuilderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DistributionBuilderLibrary.cpp
)


#### Function ####

SET(LBCPP_FUNCTION_SOURCES
  ../include/lbcpp/Function/predeclarations.h
  ../include/lbcpp/Function/ScalarFunction.h
  ../include/lbcpp/Function/ScalarVectorFunction.h
  Function/ScalarFunction.cpp
  ../include/lbcpp/Function/IterationFunction.h
  ../include/lbcpp/Function/StoppingCriterion.h
  ../include/lbcpp/Function/Predicate.h
  ../include/lbcpp/Function/Evaluator.h
  Function/Evaluator.cpp
  Function/FunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FunctionLibrary.cpp
)
 
SET(LBCPP_FUNCTION_SCALAR_SOURCES
  Function/ScalarFunction/ComposeScalarFunction.h
  Function/ScalarFunction/MultiplyByScalarFunction.h
  Function/ScalarFunction/AbsFunction.h
  Function/ScalarFunction/SquareFunction.h
  Function/ScalarFunction/AddConstantScalarFunction.h
  Function/ScalarFunction/AngleDifferenceFunction.h
  Function/ScalarFunction/ScalarFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ScalarFunctionLibrary.cpp
)

SET(LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES
  Function/ScalarVectorFunction/BinarySumScalarVectorFunction.h
  Function/ScalarVectorFunction/MultiplyByScalarVectorFunction.h
  Function/ScalarVectorFunction/ScalarVectorFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ScalarVectorFunctionLibrary.cpp
)

SET(LBCPP_FUNCTION_ITERATION_SOURCES
  Function/IterationFunction/ConstantIterationFunction.h
  Function/IterationFunction/InvLinearIterationFunction.h
  Function/IterationFunction/IterationFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/IterationFunctionLibrary.cpp
)

SET(LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES
  Function/StoppingCriterion/MaxIterationsStoppingCriterion.h
  Function/StoppingCriterion/MaxIterationsWithoutImprovementStoppingCriterion.h
  Function/StoppingCriterion/AverageImprovementStoppingCriterion.h
  Function/StoppingCriterion/LogicalOrStoppingCriterion.h
  Function/StoppingCriterion/StoppingCriterionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/StoppingCriterionLibrary.cpp
)

SET(LBCPP_FUNCTION_PREDICATE_SOURCES
  Function/Predicate/BinaryPredicate.h
  Function/Predicate/UnaryComparisonPredicate.h
  Function/Predicate/PredicateLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PredicateLibrary.cpp
)

SET(LBCPP_FUNCTION_EVALUATOR_SOURCES
  Function/Evaluator/BinaryClassificationEvaluator.h
  Function/Evaluator/ClassificationEvaluator.h
  Function/Evaluator/RegressionEvaluator.h
  Function/Evaluator/RankingEvaluator.h
  Function/Evaluator/MultiLabelClassificationEvaluator.h
  Function/Evaluator/DefaultSupervisedEvaluator.h
  Function/Evaluator/SaveToDirectoryEvaluator.h
  Function/Evaluator/Utilities.h
  Function/Evaluator/Utilities.cpp
  Function/Evaluator/ConcatenateScoreObjectFunction.h
  Function/Evaluator/ContainerSupervisedEvaluator.h
  Function/Evaluator/SymmetricMatrixSupervisedEvaluator.h
  Function/Evaluator/EvaluatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/EvaluatorLibrary.cpp
)

#### FeatureGenerator ####

GENERATE_INTROSPECTION_CODE(FeatureGenerator/Generic/GenericFeatureGeneratorLibrary.xml GenericFeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Atomic/AtomicFeatureGeneratorLibrary.xml AtomicFeatureGeneratorLibrary.cpp)

SET(LBCPP_FEATURE_GENERATOR_SOURCES
  ../include/lbcpp/FeatureGenerator/FeatureGenerator.h
  FeatureGenerator/FeatureGeneratorCallbacks.hpp
  FeatureGenerator/FeatureGenerator.cpp
  FeatureGenerator/FeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FeatureGeneratorLibrary.cpp
)

SET(LBCPP_FEATURE_GENERATOR_GENERIC_SOURCES
  FeatureGenerator/Generic/ConcatenateFeatureGenerator.h
  FeatureGenerator/Generic/ContainerWindowFeatureGenerator.h
  FeatureGenerator/Generic/MatrixWindowFeatureGenerator.h
  FeatureGenerator/Generic/CartesianProductFeatureGenerator.h
  FeatureGenerator/Generic/DynamicallyMappedFeatureGenerator.h
  FeatureGenerator/Generic/GenericFeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/GenericFeatureGeneratorLibrary.cpp
)

SET(LBCPP_FEATURE_GENERATOR_ATOMIC_SOURCES
  FeatureGenerator/Atomic/BooleanFeatureGenerator.h
  FeatureGenerator/Atomic/EnumerationFeatureGenerator.h
  FeatureGenerator/Atomic/DoubleFeatureGenerator.h
  FeatureGenerator/Atomic/DiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/HardDiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/SoftDiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/SignedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/AtomicFeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/AtomicFeatureGeneratorLibrary.cpp
)

SET(LBCPP_FEATURE_GENERATOR_COMPOSITE_SOURCES
  FeatureGenerator/Composite/EnumerationDistributionFeatureGenerator.h
  FeatureGenerator/Composite/CompositeFeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CompositeFeatureGeneratorLibrary.cpp
)

#### Learning ####

SET(LBCPP_LEARNING_SOURCES
  ../include/lbcpp/Learning/LearnableFunction.h
  ../include/lbcpp/Learning/OnlineLearner.h
  ../include/lbcpp/Learning/BatchLearner.h
  ../include/lbcpp/Learning/LossFunction.h
  ../include/lbcpp/Learning/Numerical.h
  ../include/lbcpp/Learning/DecisionTree.h
  Learning/LearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LearningLibrary.cpp
)

SET(LBCPP_LEARNING_ONLINE_LEARNER_SOURCES
  Learning/OnlineLearner/StoppingCriterionOnlineLearner.h
  Learning/OnlineLearner/RestoreBestParametersOnlineLearner.h
  Learning/OnlineLearner/EvaluatorOnlineLearner.h
  Learning/OnlineLearner/CompositeOnlineLearner.h
  Learning/OnlineLearner/HierarchicalOnlineLearner.h
  Learning/OnlineLearner/OnlineLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OnlineLearnerLibrary.cpp
)

SET(LBCPP_LEARNING_BATCH_LEARNER_SOURCES
  Learning/BatchLearner/BatchLearner.cpp
  Learning/BatchLearner/ProxyFunctionBatchLearner.h
  Learning/BatchLearner/StochasticBatchLearner.h
  Learning/BatchLearner/CompositeFunctionBatchLearner.h
  Learning/BatchLearner/UnaryHigherOrderFunctionBatchLearner.h
  Learning/BatchLearner/FilterUnsupervisedExamplesBatchLearner.h
  Learning/BatchLearner/BatchLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/BatchLearnerLibrary.cpp
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_SOURCES
  Learning/LossFunction/LossFunction.cpp
  Learning/LossFunction/LearningLossFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LearningLossFunctionLibrary.cpp
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES
  Learning/LossFunction/SquareRegressionLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES
  Learning/LossFunction/HingeDiscriminativeLossFunction.h
  Learning/LossFunction/LogBinomialDiscriminativeLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES
  Learning/LossFunction/OneAgainstAllMultiClassLossFunction.h
  Learning/LossFunction/MostViolatedMultiClassLossFunction.h
  Learning/LossFunction/LogBinomialMultiClassLossFunction.h
)

SET(LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES
  Learning/LossFunction/AdditiveRankingLossFunction.h
  Learning/LossFunction/AllPairsRankingLossFunction.h
  Learning/LossFunction/MostViolatedPairRankingLossFunction.h
  Learning/LossFunction/BestAgainstAllRankingLossFunction.h
)

SET(LBCPP_LEARNING_NUMERICAL_SOURCES
  Learning/Numerical/Numerical.cpp
  Learning/Numerical/LinearLearningMachine.h
  Learning/Numerical/NumericalLearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningLibrary.cpp
)

SET(LBCPP_LEARNING_NUMERICAL_LEARNABLE_FUNCTION_SOURCES
  Learning/Numerical/AddBiasLearnableFunction.h
  Learning/Numerical/LinearLearnableFunction.h
  Learning/Numerical/MultiLinearLearnableFunction.h
  Learning/Numerical/RankingLearnableFunction.h
)

SET(LBCPP_LEARNING_NUMERICAL_ONLINE_SOURCES
  Learning/Numerical/GradientDescentOnlineLearner.h
  Learning/Numerical/StochasticGDOnlineLearner.h
  Learning/Numerical/PerEpisodeGDOnlineLearner.h
)

SET(LBCPP_LEARNING_NUMERICAL_BATCH_SOURCES
  Learning/Numerical/AddBiasBatchLearner.h
)

SET(LBCPP_LEARNING_NUMERICAL_LIBSVM_SOURCES
  Learning/Numerical/LibSVM/libsvm.h
  Learning/Numerical/LibSVM/libsvm.cpp
  Learning/Numerical/LibSVM/LibSVMLearningMachine.h
  Learning/Numerical/LibSVM/LibSVMClassifier.h
  Learning/Numerical/LibSVM/LibSVMBinaryClassifier.h
  Learning/Numerical/LibSVM/LibSVMLearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LibSVMLearningLibrary.cpp
)

SET(LBCPP_LEARNING_DECISION_TREE_SOURCES
  Learning/DecisionTree/DecisionTree.cpp
  Learning/DecisionTree/RTreeFunction.h
  Learning/DecisionTree/RTreeFunction.cpp
  Learning/DecisionTree/RTreeBatchLearner.h
  Learning/DecisionTree/BinaryDecisionTreeFunction.h
  Learning/DecisionTree/BinaryDecisionTreeBatchLearner.h
  Learning/DecisionTree/BinaryDecisionTreeBatchLearner.cpp
  Learning/DecisionTree/DecisionTreeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeLibrary.cpp
)

SET(LBCPP_LEARNING_DECISION_TREE_DATA_SOURCES
  Learning/DecisionTree/Data/BinaryDecisionTree.h
  Learning/DecisionTree/Data/BinaryDecisionTree.cpp
  Learning/DecisionTree/Data/DecisionTreeExampleVector.h
  Learning/DecisionTree/Data/DecisionTreeExampleVector.cpp
  Learning/DecisionTree/Data/SplitScoringFunction.h
  Learning/DecisionTree/Data/SplitScoringFunction.cpp
  Learning/DecisionTree/Data/BinaryDecisionTreeSplitter.h
  Learning/DecisionTree/Data/BinaryDecisionTreeSplitter.cpp
  Learning/DecisionTree/Data/DecisionTreeDataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeDataLibrary.cpp
)

#### Optimizer ####

SET(LBCPP_OPTIMIZER_SOURCES
  ../include/lbcpp/Optimizer/Optimizer.h
  ../include/lbcpp/Optimizer/OptimizerState.h
  ../include/lbcpp/Optimizer/OptimizerContext.h
  Optimizer/Optimizer.cpp
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

SET(LBCPP_OPTIMIZER_CONTEXT_SOURCES
  Optimizer/Context/SynchroneousOptimizerContext.h
  Optimizer/Context/MultiThreadedOptimizerContext.h
  Optimizer/Context/OptimizerContextLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerContextLibrary.cpp
)

SET(LBCPP_OPTIMIZER_OPTIMIZER_SOURCES
  Optimizer/Optimizer/PopulationBasedOptimizer.h
  Optimizer/Optimizer/EDAOptimizer.h
  Optimizer/Optimizer/AsyncEDAOptimizer.h
  Optimizer/Optimizer/UniformSampleAndPickBestOptimizer.h
  Optimizer/Optimizer/OptimizerOptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerOptimizerLibrary.cpp
)

IF(LBCPP_NETWORKING)
  SET(LBCPP_OPTIMIZER_CONTEXT_NETWORK_SOURCES
    Optimizer/Context/Network/DistributedOptimizerContext.h
    Optimizer/Context/Network/DistributedOptimizerContext.cpp
    Optimizer/Context/Network/OptimizerContextNetworkLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/OptimizerContextNetworkLibrary.cpp
    ${LBCPP_OPTIMIZER_CONTEXT_NETWORK_SOURCES}
  )
ENDIF(LBCPP_NETWORKING)

#### UserInterface ####
IF(LBCPP_USER_INTERFACE)

  # Res input data.
  FILE(GLOB LBCPP_USER_INTERFACE_DATA_FILES
    UserInterface/Data/*.png
  )

  # Where is jucebinbuild?
  GET_TARGET_PROPERTY(JUCE_BIN_BUILD_COMMAND jucebinbuild LOCATION)

  # The generated files.
  SET(LBCPP_USER_INTERFACE_DATA_CPP ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.cpp)
  SET(LBCPP_USER_INTERFACE_DATA_H ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.h)

  # Main commmand for resources generation.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_CPP}
    COMMAND ${JUCE_BIN_BUILD_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/Data ${CMAKE_CURRENT_BINARY_DIR} UserInterfaceData precompiled.h
    DEPENDS ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND} 
    COMMENT "User Interface Data"
  )
  # Dummy custom command for second generated file.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_H}
    COMMAND echo
    DEPENDS  ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND}
  )

  SET(LBCPP_USER_INTEFACE_GENERATED_FILES
    ${LBCPP_USER_INTERFACE_DATA_H}
    ${LBCPP_USER_INTERFACE_DATA_CPP}
  )

  #IDE_ONLY_SOURCES(LBCPP_USER_INTEFACE_GENERATED_FILES ${LBCPP_USER_INTEFACE_GENERATED_FILES})

  SET(LBCPP_USER_INTERFACE_SOURCES
    ../include/lbcpp/UserInterface/SimpleTreeViewItem.h
    ../include/lbcpp/UserInterface/VariableSelector.h
    ../include/lbcpp/UserInterface/ComponentWithPreferedSize.h
    ../include/lbcpp/UserInterface/UserInterfaceManager.h
    ../include/lbcpp/UserInterface/ObjectEditor.h
    ../include/lbcpp/UserInterface/MatrixComponent.h
    ../include/lbcpp/UserInterface/PieChartComponent.h
    ../include/lbcpp/UserInterface/HistogramComponent.h
    UserInterface/UserInterfaceManager.cpp
    ${LBCPP_USER_INTEFACE_GENERATED_FILES}
  )

  SET(LBCPP_USER_INTERFACE_COMPONENT_SOURCES
    UserInterface/Component/ObjectEditor.cpp
    UserInterface/Component/VariableTreeView.h
    UserInterface/Component/VariableTreeView.cpp
    UserInterface/Component/ContainerTableListBox.h
    UserInterface/Component/ContainerTableListBox.cpp
    UserInterface/Component/ContainerCurveEditor.h
    UserInterface/Component/ContainerCurveEditor.cpp
    UserInterface/Component/ExecutionTraceTreeViewItem.h
    UserInterface/Component/ExecutionTraceTreeViewItem.cpp
    UserInterface/Component/ExecutionTraceTreeView.h
    UserInterface/Component/ExecutionTraceTreeView.cpp
    UserInterface/Component/ROCScoreObjectComponent.h
    UserInterface/Component/ConfusionMatrixComponent.h
    UserInterface/Component/DoubleMatrixComponent.h
    UserInterface/Component/UserInterfaceComponentLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceComponentLibrary.cpp
  )

ENDIF(LBCPP_USER_INTERFACE)

SET(LBCPP_LUA_LUA_SOURCES
  Lua/lua/lapi.c
  Lua/lua/lapi.h
  Lua/lua/lauxlib.c
  Lua/lua/lauxlib.h
  Lua/lua/lbaselib.c
  Lua/lua/lcode.c
  Lua/lua/lcode.h
  Lua/lua/ldblib.c
  Lua/lua/ldebug.c
  Lua/lua/ldebug.h
  Lua/lua/ldo.c
  Lua/lua/ldo.h
  Lua/lua/ldump.c
  Lua/lua/lfunc.c
  Lua/lua/lfunc.h
  Lua/lua/lgc.c
  Lua/lua/lgc.h
  Lua/lua/linit.c
  Lua/lua/liolib.c
  Lua/lua/llex.c
  Lua/lua/llex.h
  Lua/lua/llimits.h
  Lua/lua/lmathlib.c
  Lua/lua/lmem.c
  Lua/lua/lmem.h
  Lua/lua/loadlib.c
  Lua/lua/lobject.c
  Lua/lua/lobject.h
  Lua/lua/lopcodes.c
  Lua/lua/lopcodes.h
  Lua/lua/loslib.c
  Lua/lua/lparser.c
  Lua/lua/lparser.h
  Lua/lua/lstate.c
  Lua/lua/lstate.h
  Lua/lua/lstring.c
  Lua/lua/lstring.h
  Lua/lua/lstrlib.c
  Lua/lua/ltable.c
  Lua/lua/ltable.h
  Lua/lua/ltablib.c
  Lua/lua/ltm.c
  Lua/lua/ltm.h
  #Lua/lua/lua.c
  Lua/lua/lua.h
  #Lua/lua/luac.c
  Lua/lua/luaconf.h
  Lua/lua/lualib.h
  Lua/lua/lundump.c
  Lua/lua/lundump.h
  Lua/lua/lvm.c
  Lua/lua/lvm.h
  Lua/lua/lzio.c
  Lua/lua/lzio.h
  #Lua/lua/print.c
) 
# interpreter:	lua.c
#  compiler:	luac.c print.c



##### MAIN ####

SET(LBCPP_MAIN_SOURCES
  ../include/lbcpp/common.h
  precompiled.h
  precompiled.cpp
  ../include/lbcpp/lbcpp.h
  ../include/lbcpp/library.h
  library.cpp
  LBCppLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppLibrary.cpp
)

SET(LBCPP_SOURCES
  ${LBCPP_MAIN_SOURCES}

  # Core
  ${LBCPP_CORE_SOURCES}
    ${LBCPP_CORE_IMPL_HEADERS}
    ${LBCPP_CORE_TYPE_SOURCES}
    ${LBCPP_CORE_OBJECT_SOURCES}
    ${LBCPP_CORE_FUNCTION_SOURCES}

  # Data
  ${LBCPP_DATA_SOURCES}
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
    ${LBCPP_DATA_MATRIX_SOURCES}
  
  # Network
  ${LBCPP_NETWORK_SOURCES}
    ${LBCPP_NETWORK_GRID_SOURCES}
    ${LBCPP_NETWORK_MANAGER_SOURCES}
    ${LBCPP_NETWORK_WORK_UNIT_SOURCES}
  
  # Sampler / Distribution
  ${LBCPP_SAMPLER_SOURCES}
    ${LBCPP_SAMPLER_CONTINUOUS_SOURCES}
    ${LBCPP_SAMPLER_DISCRETE_SOURCES}
    ${LBCPP_SAMPLER_COMPOSITE_SOURCES}

  ${LBCPP_DISTRIBUTION_SOURCES}
    ${LBCPP_DISTRIBUTION_FUNCTION_SOURCES}
    ${LBCPP_DISTRIBUTION_BUILDER_SOURCES}  

  # Execution
  ${LBCPP_EXECUTION_SOURCES}
    ${LBCPP_EXECUTION_CALLBACK_SOURCES}
    ${LBCPP_EXECUTION_CONTEXT_SOURCES}
  
  # Function
  ${LBCPP_FUNCTION_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES}
    ${LBCPP_FUNCTION_ITERATION_SOURCES}
    ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES}
    ${LBCPP_FUNCTION_PREDICATE_SOURCES}
    ${LBCPP_FUNCTION_EVALUATOR_SOURCES}

  # FeatureGenerator
  ${LBCPP_FEATURE_GENERATOR_SOURCES}
    ${LBCPP_FEATURE_GENERATOR_GENERIC_SOURCES}
    ${LBCPP_FEATURE_GENERATOR_ATOMIC_SOURCES}
    ${LBCPP_FEATURE_GENERATOR_COMPOSITE_SOURCES}

  # Learning
  ${LBCPP_LEARNING_SOURCES}
    ${LBCPP_LEARNING_ONLINE_LEARNER_SOURCES}
    ${LBCPP_LEARNING_BATCH_LEARNER_SOURCES}
    ${LBCPP_LEARNING_LOSS_FUNCTION_SOURCES}
      ${LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES}
      ${LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES}
      ${LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES}
      ${LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES}

    ${LBCPP_LEARNING_NUMERICAL_SOURCES}
      ${LBCPP_LEARNING_NUMERICAL_LEARNABLE_FUNCTION_SOURCES}
      ${LBCPP_LEARNING_NUMERICAL_ONLINE_SOURCES}
      ${LBCPP_LEARNING_NUMERICAL_BATCH_SOURCES}
      ${LBCPP_LEARNING_NUMERICAL_LIBSVM_SOURCES}
    
    ${LBCPP_LEARNING_DECISION_TREE_SOURCES}
      ${LBCPP_LEARNING_DECISION_TREE_DATA_SOURCES}

  # Optimizer
  ${LBCPP_OPTIMIZER_SOURCES}
    ${LBCPP_OPTIMIZER_CONTEXT_SOURCES}
    ${LBCPP_OPTIMIZER_CONTEXT_NETWORK_SOURCES}
    ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES}
    ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES}
    ${LBCPP_OPTIMIZER_OLD_SOURCES}
    
  # Lua
  ${LBCPP_LUA_LUA_SOURCES}
    
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SOURCES)

## Main
SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Core
SOURCE_GROUP(Core FILES ${LBCPP_CORE_SOURCES})
SOURCE_SUB_GROUP(Core impl FILES ${LBCPP_CORE_IMPL_HEADERS})
SOURCE_SUB_GROUP(Core Type FILES ${LBCPP_CORE_TYPE_SOURCES})
SOURCE_SUB_GROUP(Core Object FILES ${LBCPP_CORE_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Core Function FILES ${LBCPP_CORE_FUNCTION_SOURCES})

## Data
SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})
SOURCE_SUB_GROUP(Data Matrix FILES ${LBCPP_DATA_MATRIX_SOURCES})

## Network
SOURCE_GROUP(Network FILES ${LBCPP_NETWORK_SOURCES})
SOURCE_SUB_GROUP(Network Grid FILES ${LBCPP_NETWORK_GRID_SOURCES})
SOURCE_SUB_GROUP(Network Manager FILES ${LBCPP_NETWORK_MANAGER_SOURCES})
SOURCE_SUB_GROUP(Network WorkUnit FILES ${LBCPP_NETWORK_WORK_UNIT_SOURCES})

## Sampler / Distribution
SOURCE_GROUP(Sampler FILES ${LBCPP_SAMPLER_SOURCES})
SOURCE_SUB_GROUP(Sampler Continuous FILES ${LBCPP_SAMPLER_CONTINUOUS_SOURCES})
SOURCE_SUB_GROUP(Sampler Discrete FILES ${LBCPP_SAMPLER_DISCRETE_SOURCES})
SOURCE_SUB_GROUP(Sampler Composite FILES ${LBCPP_SAMPLER_COMPOSITE_SOURCES})
    
SOURCE_GROUP(Distribution FILES ${LBCPP_DISTRIBUTION_SOURCES})
SOURCE_SUB_GROUP(Distribution Function FILES ${LBCPP_DISTRIBUTION_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Distribution Builder FILES ${LBCPP_DISTRIBUTION_BUILDER_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${LBCPP_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${LBCPP_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${LBCPP_EXECUTION_CONTEXT_SOURCES})

## Function
SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function ScalarFunction FILES ${LBCPP_FUNCTION_SCALAR_SOURCES})
SOURCE_SUB_GROUP(Function ScalarVectorFunction FILES ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Function IterationFunction FILES ${LBCPP_FUNCTION_ITERATION_SOURCES})
SOURCE_SUB_GROUP(Function StoppingCriterion FILES ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES})
SOURCE_SUB_GROUP(Function Predicate FILES ${LBCPP_FUNCTION_PREDICATE_SOURCES})
SOURCE_SUB_GROUP(Function Evaluator FILES ${LBCPP_FUNCTION_EVALUATOR_SOURCES})

## FeatureGenerator
SOURCE_GROUP(FeatureGenerator FILES ${LBCPP_FEATURE_GENERATOR_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator Generic FILES ${LBCPP_FEATURE_GENERATOR_GENERIC_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator Atomic FILES ${LBCPP_FEATURE_GENERATOR_ATOMIC_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator Composite FILES ${LBCPP_FEATURE_GENERATOR_COMPOSITE_SOURCES})

## Learning
SOURCE_GROUP(Learning FILES ${LBCPP_LEARNING_SOURCES})
SOURCE_SUB_GROUP(Learning OnlineLearner FILES ${LBCPP_LEARNING_ONLINE_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Learning BatchLearner FILES ${LBCPP_LEARNING_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Learning LossFunction FILES ${LBCPP_LEARNING_LOSS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP2(Learning LossFunction Regression FILES ${LBCPP_LEARNING_LOSS_FUNCTION_REGRESSION_SOURCES})
SOURCE_SUB_GROUP2(Learning LossFunction Discriminative FILES ${LBCPP_LEARNING_LOSS_FUNCTION_DISCRIMINATIVE_SOURCES})
SOURCE_SUB_GROUP2(Learning LossFunction MultiClass FILES ${LBCPP_LEARNING_LOSS_FUNCTION_MULTI_CLASS_SOURCES})
SOURCE_SUB_GROUP2(Learning LossFunction Ranking FILES ${LBCPP_LEARNING_LOSS_FUNCTION_RANKING_SOURCES})
SOURCE_SUB_GROUP(Learning Numerical FILES ${LBCPP_LEARNING_NUMERICAL_SOURCES})
SOURCE_SUB_GROUP2(Learning Numerical LearnableFunction FILES ${LBCPP_LEARNING_NUMERICAL_LEARNABLE_FUNCTION_SOURCES})
SOURCE_SUB_GROUP2(Learning Numerical OnlineLearner FILES ${LBCPP_LEARNING_NUMERICAL_ONLINE_SOURCES})
SOURCE_SUB_GROUP2(Learning Numerical BatchLearner FILES ${LBCPP_LEARNING_NUMERICAL_BATCH_SOURCES})
SOURCE_SUB_GROUP2(Learning Numerical LibSVM FILES ${LBCPP_LEARNING_NUMERICAL_LIBSVM_SOURCES})
SOURCE_SUB_GROUP(Learning DecisionTree FILES ${LBCPP_LEARNING_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP2(Learning DecisionTree Data FILES ${LBCPP_LEARNING_DECISION_TREE_DATA_SOURCES})

## Optimizer
SOURCE_GROUP(Optimizer FILES ${LBCPP_OPTIMIZER_SOURCES})
SOURCE_SUB_GROUP(Optimizer Context FILES ${LBCPP_OPTIMIZER_CONTEXT_SOURCES} ${LBCPP_OPTIMIZER_CONTEXT_NETWORK_SOURCES})
SOURCE_SUB_GROUP(Optimizer ObjectiveFunction FILES ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Optimizer Optimizer FILES ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES})

## Lua
SOURCE_SUB_GROUP(Lua Lua FILES ${LBCPP_LUA_LUA_SOURCES})

## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-core${NAME_FILE_SUFFIX} SHARED ${LBCPP_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-core${NAME_FILE_SUFFIX} ${LBCPP_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-core${NAME_FILE_SUFFIX} juce${NAME_FILE_SUFFIX})
