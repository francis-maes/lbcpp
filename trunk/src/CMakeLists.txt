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
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

IF(LBCPP_NETWORKING)
  GENERATE_INTROSPECTION_CODE(Network/NetworkLibrary.xml NetworkLibrary.cpp)
ENDIF(LBCPP_NETWORKING)

GENERATE_INTROSPECTION_CODE(Distribution/DistributionLibrary.xml DistributionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Distribution/Function/DistributionFunctionLibrary.xml DistributionFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Distribution/Builder/DistributionBuilderLibrary.xml DistributionBuilderLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/MiscFunctions/MiscFunctionsLibrary.xml MiscFunctionsLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionLibrary.xml ScalarFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarVectorFunction/ScalarVectorFunctionLibrary.xml ScalarVectorFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/IterationFunction/IterationFunctionLibrary.xml IterationFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/StoppingCriterion/StoppingCriterionLibrary.xml StoppingCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Predicate/PredicateLibrary.xml PredicateLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Evaluator/EvaluatorLibrary.xml EvaluatorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(FeatureGenerator/FeatureGeneratorLibrary.xml FeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Generic/GenericFeatureGeneratorLibrary.xml GenericFeatureGeneratorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(FeatureGenerator/Atomic/AtomicFeatureGeneratorLibrary.xml AtomicFeatureGeneratorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Learning/LearningLibrary.xml LearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/OnlineLearner/OnlineLearnerLibrary.xml OnlineLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/BatchLearner/BatchLearnerLibrary.xml BatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/Numerical/NumericalLearningLibrary.xml NumericalLearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/DecisionTree/DecisionTreeLibrary.xml DecisionTreeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/DecisionTree/Data/DecisionTreeDataLibrary.xml DecisionTreeDataLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Learning/LossFunction/LearningLossFunctionLibrary.xml LearningLossFunctionLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/ObjectiveFunction/ObjectiveFunctionLibrary.xml ObjectiveFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/Optimizer/OptimizerOptimizerLibrary.xml OptimizerOptimizerLibrary.cpp)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

### old ->
GENERATE_INTROSPECTION_CODE(Perception/PerceptionLibrary.xml PerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Simple/SimplePerceptionLibrary.xml SimplePerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Modifier/ModifierPerceptionLibrary.xml ModifierPerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Container/ContainerPerceptionLibrary.xml ContainerPerceptionLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Inference/InferenceLibrary.xml InferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceCallback/InferenceCallbackLibrary.xml InferenceCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ReductionInference/ReductionInferenceLibrary.xml ReductionInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/MetaInference/MetaInferenceLibrary.xml MetaInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/BatchLearner/InferenceBatchLearnerLibrary.xml InferenceBatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/OnlineLearner/InferenceOnlineLearnerLibrary.xml InferenceOnlineLearnerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(NumericalLearning/OldNumericalLearningLibrary.xml OldNumericalLearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/LossFunction/NumericalLearningLossFunctionLibrary.xml NumericalLearningLossFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Perception/NumericalLearningPerceptionLibrary.xml NumericalLearningPerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Inference/NumericalLearningInferenceLibrary.xml NumericalLearningInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/BatchLearner/NumericalLearningBatchLearnerLibrary.xml NumericalLearningBatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/OnlineLearner/NumericalLearningOnlineLearnerLibrary.xml NumericalLearningOnlineLearnerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(DecisionTree/OldDecisionTreeLibrary.xml OldDecisionTreeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(DecisionTree/Inference/DecisionTreeInferenceLibrary.xml DecisionTreeInferenceLibrary.cpp)
### <- old

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
  Core/Function/LoadFromFileFunction.h
  Core/Function/MapContainerFunction.h
  Core/Function/SignedScalarToProbabilityFunction.h
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
  ../include/lbcpp/Data/SymmetricMatrix.h
  Data/SymmetricMatrix.cpp
  ../include/lbcpp/Data/RandomGenerator.h
  Data/RandomGenerator.cpp  
  ../include/lbcpp/Data/RandomVariable.h  
  ../include/lbcpp/Data/Consumer.h
  Data/Consumer.cpp
  ../include/lbcpp/Data/Cache.h
  Data/Cache.cpp
  Data/DataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DataLibrary.cpp
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
    ../include/lbcpp/Network/NetworkRequest.h
    Network/NetworkRequest.cpp
    ../include/lbcpp/Network/NetworkInterface.h
    Network/NetworkInterface.cpp
    Network/NetworkNotification.h
    Network/NetworkNotification.cpp
    Network/NetworkWorkUnit.h
    Network/NetworkWorkUnit.cpp
    Network/NetworkLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/NetworkLibrary.cpp
)
ENDIF(LBCPP_NETWORKING)

#### Distribution ####

SET(LBCPP_DISTRIBUTION_SOURCES
  ../include/lbcpp/Distribution/Distribution.h
  ../include/lbcpp/Distribution/DiscreteDistribution.h
  Distribution/DiscreteDistribution.cpp
  ../include/lbcpp/Distribution/ContinuousDistribution.h
  Distribution/ContinuousDistribution.cpp
  ../include/lbcpp/Distribution/MultiVariateDistribution.h
  Distribution/MultiVariateDistribution.cpp
  ../include/lbcpp/Distribution/DistributionBuilder.h
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
  Distribution/Builder/EnumerationDistributionBuilder.h
  Distribution/Builder/GaussianDistributionBuilder.h
  Distribution/Builder/BernoulliDistributionBuilder.h
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
 
SET(LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES
  Function/MiscFunctions/SelectVariableFunctions.h
  Function/MiscFunctions/IdentityFunction.h
  Function/MiscFunctions/ComposeFunction.h
  Function/MiscFunctions/MultiplyFunction.h
  Function/MiscFunctions/SetFieldFunction.h
  Function/MiscFunctions/MiscFunctionsLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MiscFunctionsLibrary.cpp
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
  Function/Evaluator/ContainerElementsEvaluator.h
  Function/Evaluator/BinaryClassificationConfusionEvaluator.h
  Function/Evaluator/ClassificationAccuracyEvaluator.h
  Function/Evaluator/MultiLabelClassificationEvaluator.h
  Function/Evaluator/RegressionErrorEvaluator.h
  Function/Evaluator/SaveToDirectoryEvaluator.h
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
  FeatureGenerator/Generic/WindowFeatureGenerator.h
  FeatureGenerator/Generic/GenericFeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/GenericFeatureGeneratorLibrary.cpp
)

SET(LBCPP_FEATURE_GENERATOR_ATOMIC_SOURCES
  FeatureGenerator/Atomic/BooleanFeatureGenerator.h
  FeatureGenerator/Atomic/EnumerationFeatureGenerator.h
  FeatureGenerator/Atomic/EnumerationDistributionFeatureGenerator.h
  FeatureGenerator/Atomic/DiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/HardDiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/SoftDiscretizedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/SignedNumberFeatureGenerator.h
  FeatureGenerator/Atomic/AtomicFeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/AtomicFeatureGeneratorLibrary.cpp
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
  Learning/OnlineLearner/OnlineLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OnlineLearnerLibrary.cpp
)

SET(LBCPP_LEARNING_BATCH_LEARNER_SOURCES
  Learning/BatchLearner/ProxyFunctionBatchLearner.h
  Learning/BatchLearner/StochasticBatchLearner.h
  Learning/BatchLearner/CompositeFunctionBatchLearner.h
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
  Learning/Numerical/GradientDescentOnlineLearner.h
  Learning/Numerical/StochasticGDOnlineLearner.h
  Learning/Numerical/PerEpisodeGDOnlineLearner.h
  Learning/Numerical/LinearLearnableFunction.h
  Learning/Numerical/MultiLinearLearnableFunction.h
  Learning/Numerical/LinearLearningMachine.h
  Learning/Numerical/NumericalLearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningLibrary.cpp
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
  ../include/lbcpp/Optimizer/OptimizerInput.h
  ../include/lbcpp/Optimizer/Optimizer.h
  ../include/lbcpp/Optimizer/ObjectiveFunction.h
  Optimizer/OptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerLibrary.cpp
)

SET(LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES
  Optimizer/ObjectiveFunction/MarginalObjectiveFunction.h
  Optimizer/ObjectiveFunction/ObjectiveFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ObjectiveFunctionLibrary.cpp
)

SET(LBCPP_OPTIMIZER_OPTIMIZER_SOURCES
  Optimizer/Optimizer/UniformSampleAndPickBestOptimizer.h
  Optimizer/Optimizer/IterativeBracketingOptimizer.h
  Optimizer/Optimizer/OptimizerOptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OptimizerOptimizerLibrary.cpp
)

#### Perception (Old) ####

SET(LBCPP_PERCEPTION_SOURCES
  ../include/lbcpp/Perception/Perception.h
  Perception/Perception.cpp
  ../include/lbcpp/Perception/PerceptionRewriter.h
  Perception/PerceptionRewriter.cpp
  Perception/PerceptionRewriterRules.h
  Perception/PerceptionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PerceptionLibrary.cpp
)

SET(LBCPP_PERCEPTION_SIMPLE_SOURCES
  Perception/Simple/NullPerception.h
  Perception/Simple/IdentityPerception.h
  Perception/Simple/FunctionBasedPerception.h
  Perception/Simple/SimplePerceptionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SimplePerceptionLibrary.cpp
)

SET(LBCPP_PERCEPTION_MODIFIER_SOURCES
  Perception/Modifier/RewritedPerception.h
  Perception/Modifier/ProductPerception.h
  Perception/Modifier/ProductPerception.cpp
  Perception/Modifier/ComposePerception.h
  Perception/Modifier/FlattenPerception.h
  Perception/Modifier/CollapsePerception.h
  Perception/Modifier/SelectAndMakeProductsPerception.h
  Perception/Modifier/ModifierPerceptionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ModifierPerceptionLibrary.cpp
)

SET(LBCPP_PERCEPTION_CONTAINER_SOURCES
  Perception/Container/WindowPerception.h
  Perception/Container/HistogramPerception.h
  Perception/Container/HistogramPerception.cpp
  Perception/Container/SegmentsPerception.h
  Perception/Container/SegmentsPerception.cpp
  Perception/Container/BoundsProximityPerception.h
  Perception/Container/DistributionPerception.h
  Perception/Container/InputLabelPairPerception.h
  Perception/Container/ConcatenatePairPerception.h
  Perception/Container/ContainerPerceptionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerPerceptionLibrary.cpp
)

#### Inference (Old) ####

SET(LBCPP_INFERENCE_SOURCES
  ../include/lbcpp/Inference/predeclarations.h
  ../include/lbcpp/Inference/Inference.h
  Inference/Inference.cpp
  ../include/lbcpp/Inference/DecoratorInference.h
  Inference/DecoratorInference.cpp
  ../include/lbcpp/Inference/SequentialInference.h
  Inference/SequentialInference.cpp
  ../include/lbcpp/Inference/ParallelInference.h
  Inference/ParallelInference.cpp
  ../include/lbcpp/Inference/InferenceCallback.h
  ../include/lbcpp/Inference/InferenceResultCache.h
  Inference/InferenceCallback.cpp
  ../include/lbcpp/Inference/InferenceBatchLearner.h
  Inference/InferenceBatchLearner.cpp
  ../include/lbcpp/Inference/InferenceOnlineLearner.h
  Inference/InferenceOnlineLearner.cpp
  Inference/InferenceLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceLibrary.cpp
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  Inference/InferenceCallback/CacheInferenceCallback.h
  Inference/InferenceCallback/EvaluationInferenceCallback.h
  Inference/InferenceCallback/InferenceCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceCallbackLibrary.cpp  
)

SET(LBCPP_INFERENCE_REDUCTION_SOURCES
  Inference/ReductionInference/OneAgainstAllClassificationInference.h
  Inference/ReductionInference/OneAgainstOneClassificationInference.h
  Inference/ReductionInference/OneAgainstAllMultiLabelClassificationInference.h
  Inference/ReductionInference/RankingBasedClassificationInference.h
  Inference/ReductionInference/ParallelVoteInference.h
  Inference/ReductionInference/SharedParallelVectorInference.h
  Inference/ReductionInference/ReductionInferenceLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ReductionInferenceLibrary.cpp
)

SET(LBCPP_INFERENCE_META_SOURCES
  Inference/MetaInference/CallbackBasedDecoratorInference.h
  Inference/MetaInference/RunOnSupervisedExamplesInference.h
  Inference/MetaInference/EvaluationInference.h
  Inference/MetaInference/CrossValidationInference.h
  Inference/MetaInference/MetaInferenceLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MetaInferenceLibrary.cpp
)

SET(LBCPP_INFERENCE_BATCH_LEARNER_SOURCES
  Inference/BatchLearner/DummyInferenceLearner.h
  Inference/BatchLearner/StaticSequentialInferenceLearner.h
  Inference/BatchLearner/StaticParallelInferenceLearner.h
  Inference/BatchLearner/SharedParallelInferenceLearner.h
  Inference/BatchLearner/DecoratorInferenceLearner.h
  Inference/BatchLearner/StochasticInferenceLearner.h
  Inference/BatchLearner/InitializeByCloningInferenceLearner.h
  Inference/BatchLearner/PrecomputePerceptionsInferenceLearner.h
  Inference/BatchLearner/AutoTuneInferenceLearner.h
  Inference/BatchLearner/InferenceBatchLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceBatchLearnerLibrary.cpp
)

SET(LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES
  Inference/OnlineLearner/ComputeScoreOnlineLearner.h
  Inference/OnlineLearner/ComputeEvaluatorOnlineLearner.h
  Inference/OnlineLearner/RandomizerOnlineLearner.h
  Inference/OnlineLearner/StoppingCriterionOnlineLearner.h
  Inference/OnlineLearner/SaveScoresToGnuPlotFileOnlineLearner.h
  Inference/OnlineLearner/InferenceOnlineLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceOnlineLearnerLibrary.cpp
)

#### NumericalLearning ####

SET(LBCPP_NUMERICAL_LEARNING_SOURCES
  ../include/lbcpp/NumericalLearning/LossFunctions.h
  NumericalLearning/LossFunctions.cpp
  ../include/lbcpp/NumericalLearning/NumericalInference.h
  ../include/lbcpp/NumericalLearning/NumericalSupervisedInference.h
  ../include/lbcpp/NumericalLearning/NumericalLearning.h
  NumericalLearning/NumericalLearning.cpp
  NumericalLearning/OldNumericalLearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OldNumericalLearningLibrary.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_SOURCES
  NumericalLearning/LossFunction/BinaryClassificationRankingLossFunction.h
  NumericalLearning/LossFunction/L2RegularizerFunction.h
  NumericalLearning/LossFunction/NumericalLearningLossFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningLossFunctionLibrary.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_MATH_SOURCES
  NumericalLearning/Math/DoubleConstUnaryOperation.h
  NumericalLearning/Math/DoubleUnaryOperation.h
  NumericalLearning/Math/DoubleDotProductOperation.h
  NumericalLearning/Math/DoubleAssignmentOperation.h
)

SET(LBCPP_NUMERICAL_LEARNING_PERCEPTION_SOURCES
  NumericalLearning/Perception/BooleanFeatures.h
  NumericalLearning/Perception/AddUnitFeatures.h
  NumericalLearning/Perception/EnumValueFeatures.h
  NumericalLearning/Perception/DiscretizedNumberFeatures.h
  NumericalLearning/Perception/HardDiscretizedNumberFeatures.h
  NumericalLearning/Perception/SoftDiscretizedNumberFeatures.h
  NumericalLearning/Perception/SignedNumberFeatures.h
  NumericalLearning/Perception/NumericalLearningPerceptionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningPerceptionLibrary.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_INFERENCE_SOURCES
  NumericalLearning/Inference/LinearInference.h
  NumericalLearning/Inference/MultiLinearInference.h
  NumericalLearning/Inference/BinaryClassificationInference.h
  NumericalLearning/Inference/RegressionInference.h
  NumericalLearning/Inference/MultiClassInference.h
  NumericalLearning/Inference/RankingInference.h
  NumericalLearning/Inference/AddBiasInference.h
  NumericalLearning/Inference/TransferFunctionDecoratorInference.h
  NumericalLearning/Inference/NumericalLearningInferenceLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningInferenceLibrary.cpp  
)

SET(LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_SOURCES
  NumericalLearning/BatchLearner/PrecomputePerceptionsNumericalInferenceLearner.h
  NumericalLearning/BatchLearner/AddBiasInferenceLearner.h
  NumericalLearning/BatchLearner/NumericalLearningBatchLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningBatchLearnerLibrary.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_SOURCES
  NumericalLearning/OnlineLearner/GradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/GradientDescentOnlineLearner.cpp
  NumericalLearning/OnlineLearner/StochasticGradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/BatchGradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/GraftingOnlineLearner.h
  NumericalLearning/OnlineLearner/GraftingOnlineLearner.cpp
  NumericalLearning/OnlineLearner/NumericalLearningOnlineLearnerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningOnlineLearnerLibrary.cpp
)

#### DecisionTree ####

SET(LBCPP_DECISION_TREE_SOURCES
  ../include/lbcpp/DecisionTree/predeclarations.h
  ../include/lbcpp/DecisionTree/DecisionTree.h
  ../include/lbcpp/DecisionTree/RTreeInference.h
  DecisionTree/RTreeInference.cpp
  DecisionTree/RTreeInferenceLearner.h
  DecisionTree/OldDecisionTreeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OldDecisionTreeLibrary.cpp
)

SET(LBCPP_DECISION_TREE_INFERENCE_SOURCES
  DecisionTree/Inference/ExtraTreeInference.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.cpp
  DecisionTree/Inference/DecisionTreeInferenceLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeInferenceLibrary.cpp  
)

SET(LBCPP_DECISION_TREE_RTREE_SOURCES
  DecisionTree/RTree/f-table.c
  DecisionTree/RTree/tree-density.c
  DecisionTree/RTree/tree-kernel.c
  DecisionTree/RTree/tree-model.h
  DecisionTree/RTree/tree-model.c
  DecisionTree/RTree/tree-multiregr.c
  DecisionTree/RTree/tree-ok3.c
)

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
    COMMAND ${JUCE_BIN_BUILD_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/Data ${CMAKE_CURRENT_BINARY_DIR} UserInterfaceData lbcpp/common.h
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
    UserInterface/Component/UserInterfaceComponentLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceComponentLibrary.cpp
  )

ENDIF(LBCPP_USER_INTERFACE)


##### MAIN ####

SET(LBCPP_MAIN_SOURCES
  ../include/lbcpp/common.h
  ../include/lbcpp/lbcpp.h
  ../include/lbcpp/library.h
  library.cpp
  LBCppLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppLibrary.cpp
)

SET(LBCPP_CORE
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
  
  # Network
  ${LBCPP_NETWORK_SOURCES}
  
  # Distribution
  ${LBCPP_DISTRIBUTION_SOURCES}
    ${LBCPP_DISTRIBUTION_FUNCTION_SOURCES}
    ${LBCPP_DISTRIBUTION_BUILDER_SOURCES}  

  # Execution
  ${LBCPP_EXECUTION_SOURCES}
    ${LBCPP_EXECUTION_CALLBACK_SOURCES}
    ${LBCPP_EXECUTION_CONTEXT_SOURCES}
  
  # Function
  ${LBCPP_FUNCTION_SOURCES}
    ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES}
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
    ${LBCPP_LEARNING_DECISION_TREE_SOURCES}
      ${LBCPP_LEARNING_DECISION_TREE_DATA_SOURCES}

  # Optimizer
  ${LBCPP_OPTIMIZER_SOURCES}
    ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES}
    ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES}
    
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
    
  ######### old --> ############
  
  # Perception
  ${LBCPP_PERCEPTION_SOURCES}
    ${LBCPP_PERCEPTION_SIMPLE_SOURCES}
    ${LBCPP_PERCEPTION_MODIFIER_SOURCES}
    ${LBCPP_PERCEPTION_CONTAINER_SOURCES}

  # Inference
  ${LBCPP_INFERENCE_SOURCES}
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
  ${LBCPP_INFERENCE_INFERENCE_SOURCES}
    ${LBCPP_INFERENCE_REDUCTION_SOURCES}
    ${LBCPP_INFERENCE_META_SOURCES}
  ${LBCPP_INFERENCE_BATCH_LEARNER_SOURCES}
  ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES}

  # NumericalLearning
  ${LBCPP_NUMERICAL_LEARNING_SOURCES}
    ${LBCPP_NUMERICAL_LEARNING_MATH_SOURCES}
    ${LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_SOURCES}
    ${LBCPP_NUMERICAL_LEARNING_PERCEPTION_SOURCES}
    ${LBCPP_NUMERICAL_LEARNING_INFERENCE_SOURCES}
	${LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_SOURCES}
    ${LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_SOURCES}

  # DecisionTree
  ${LBCPP_DECISION_TREE_SOURCES}
    ${LBCPP_DECISION_TREE_INFERENCE_SOURCES}
  #  ${LBCPP_DECISION_TREE_RTREE_SOURCES}
  
  ######### <-- old  ############
)

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

## Network
SOURCE_GROUP(Network FILES ${LBCPP_NETWORK_SOURCES})

## Distribution
SOURCE_GROUP(Distribution FILES ${LBCPP_DISTRIBUTION_SOURCES})
SOURCE_SUB_GROUP(Distribution Function FILES ${LBCPP_DISTRIBUTION_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Distribution Builder FILES ${LBCPP_DISTRIBUTION_BUILDER_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${LBCPP_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${LBCPP_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${LBCPP_EXECUTION_CONTEXT_SOURCES})

## Function
SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function MiscFunctions FILES ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES})
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
SOURCE_SUB_GROUP(Learning DecisionTree FILES ${LBCPP_LEARNING_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP2(Learning DecisionTree Data FILES ${LBCPP_LEARNING_DECISION_TREE_DATA_SOURCES})

## Optimizer
SOURCE_GROUP(Optimizer FILES ${LBCPP_OPTIMIZER_SOURCES})
SOURCE_SUB_GROUP(Optimizer ObjectiveFunction FILES ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Optimizer Optimizer FILES ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES})

## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

######### old --> ############

## Perception
SOURCE_GROUP("Old - Perception" FILES ${LBCPP_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP("Old - Perception" Simple FILES ${LBCPP_PERCEPTION_SIMPLE_SOURCES})
SOURCE_SUB_GROUP("Old - Perception" Modifier FILES ${LBCPP_PERCEPTION_MODIFIER_SOURCES})
SOURCE_SUB_GROUP("Old - Perception" Container FILES ${LBCPP_PERCEPTION_CONTAINER_SOURCES})

## Inference
SOURCE_GROUP("Old - Inference" FILES ${LBCPP_INFERENCE_SOURCES})
SOURCE_SUB_GROUP("Old - Inference" Callback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_SUB_GROUP("Old - Inference" ReductionInference FILES ${LBCPP_INFERENCE_REDUCTION_SOURCES})
SOURCE_SUB_GROUP("Old - Inference" MetaInference FILES ${LBCPP_INFERENCE_META_SOURCES})
SOURCE_SUB_GROUP("Old - Inference" BatchLearner FILES ${LBCPP_INFERENCE_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP("Old - Inference" OnlineLearner FILES ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES})

## NumericalLearning
SOURCE_GROUP("Old - NumericalLearning" FILES ${LBCPP_NUMERICAL_LEARNING_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" Math FILES ${LBCPP_NUMERICAL_LEARNING_MATH_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" LossFunction FILES ${LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" Perception FILES ${LBCPP_NUMERICAL_LEARNING_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" Inference FILES ${LBCPP_NUMERICAL_LEARNING_INFERENCE_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" BatchLearner FILES ${LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP("Old - NumericalLearning" OnlineLearner FILES ${LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_SOURCES})

## DecisionTree
SOURCE_GROUP("Old - DecisionTree" FILES ${LBCPP_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP("Old - DecisionTree" Inference FILES ${LBCPP_DECISION_TREE_INFERENCE_SOURCES})
SOURCE_SUB_GROUP("Old - DecisionTree" RTree FILES ${LBCPP_DECISION_TREE_RTREE_SOURCES})

######### <-- old  ############

IF (UNIX)
  ADD_LIBRARY(lbcpp-core${NAME_FILE_SUFFIX} SHARED ${LBCPP_CORE})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-core${NAME_FILE_SUFFIX} ${LBCPP_CORE})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-core${NAME_FILE_SUFFIX} juce${NAME_FILE_SUFFIX})
