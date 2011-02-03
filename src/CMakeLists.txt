INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppLibrary.xml LBCppLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Core/Type/TypeLibrary.xml TypeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/CoreLibrary.xml CoreLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Data/Stream/StreamLibrary.xml StreamLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerLibrary.xml ContainerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Network/NetworkLibrary.xml NetworkLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Distribution/DistributionLibrary.xml DistributionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Distribution/Builder/DistributionBuilderLibrary.xml DistributionBuilderLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionLibrary.xml FunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/MiscFunctions/MiscFunctionsLibrary.xml MiscFunctionsLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionLibrary.xml ScalarFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarObjectFunction/ScalarObjectFunctionLibrary.xml ScalarObjectFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/IterationFunction/IterationFunctionLibrary.xml IterationFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/StoppingCriterion/StoppingCriterionLibrary.xml StoppingCriterionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Predicate/PredicateLibrary.xml PredicateLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Function/Evaluator/EvaluatorLibrary.xml EvaluatorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Operator/OperatorLibrary.xml OperatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Operator/Container/ContainerOperatorLibrary.xml ContainerOperatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Operator/FeatureGenerator/FeatureGeneratorLibrary.xml FeatureGeneratorLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Perception/PerceptionLibrary.xml PerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Simple/SimplePerceptionLibrary.xml SimplePerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Modifier/ModifierPerceptionLibrary.xml ModifierPerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Container/ContainerPerceptionLibrary.xml ContainerPerceptionLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Optimizer/OptimizerLibrary.xml OptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/ObjectiveFunction/ObjectiveFunctionLibrary.xml ObjectiveFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Optimizer/Optimizer/OptimizerOptimizerLibrary.xml OptimizerOptimizerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Inference/InferenceLibrary.xml InferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceCallback/InferenceCallbackLibrary.xml InferenceCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ReductionInference/ReductionInferenceLibrary.xml ReductionInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/MetaInference/MetaInferenceLibrary.xml MetaInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/BatchLearner/InferenceBatchLearnerLibrary.xml InferenceBatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Inference/OnlineLearner/InferenceOnlineLearnerLibrary.xml InferenceOnlineLearnerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(NumericalLearning/NumericalLearningLibrary.xml NumericalLearningLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/LossFunction/NumericalLearningLossFunctionLibrary.xml NumericalLearningLossFunctionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Perception/NumericalLearningPerceptionLibrary.xml NumericalLearningPerceptionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Inference/NumericalLearningInferenceLibrary.xml NumericalLearningInferenceLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/BatchLearner/NumericalLearningBatchLearnerLibrary.xml NumericalLearningBatchLearnerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/OnlineLearner/NumericalLearningOnlineLearnerLibrary.xml NumericalLearningOnlineLearnerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(DecisionTree/DecisionTreeLibrary.xml DecisionTreeLibrary.cpp)
GENERATE_INTROSPECTION_CODE(DecisionTree/Inference/DecisionTreeInferenceLibrary.xml DecisionTreeInferenceLibrary.cpp)

GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)

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
  Core/Type/TypeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/TypeLibrary.cpp
)

SET(LBCPP_CORE_OBJECT_SOURCES
  Core/Object/SparseVectorHelper.h
  Core/Object/SparseGenericObject.h
  Core/Object/SparseDoubleObject.h
  Core/Object/DenseGenericObject.h
  Core/Object/DenseDoubleObject.h
  Core/Object/DenseObjectObject.h
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
  Data/Stream/ClassificationDataTextParser.h
  Data/Stream/MultiLabelClassificationDataTextParser.h
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
  Execution/Callback/UserInterfaceExecutionCallback.h
  Execution/Callback/ExecutionCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackLibrary.cpp
)

SET(LBCPP_EXECUTION_CONTEXT_SOURCES
  Execution/Context/SingleThreadedExecutionContext.h
  Execution/Context/SubExecutionContext.h
  Execution/Context/MultiThreadedExecutionContext.h
  Execution/Context/ExecutionContextLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionContextLibrary.cpp
)

### Network ###

SET(LBCPP_NETWORK_SOURCES
  ../include/lbcpp/Network/NetworkClient.h
  Network/NetworkClient.cpp
  ../include/lbcpp/Network/NetworkServer.h
  Network/NetworkServer.cpp
  ../include/lbcpp/Network/NetworkCallback.h
  Network/NetworkLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NetworkLibrary.cpp
)

#### Distribution ####

SET(LBCPP_PROBABILITY_DISTRIBUTION_SOURCES
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

SET(LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_SOURCES
  Distribution/Builder/EnumerationDistributionBuilder.h
  Distribution/Builder/GaussianDistributionBuilder.h
  Distribution/Builder/BernoulliDistributionBuilder.h
  Distribution/Builder/DistributionBuilderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DistributionBuilderLibrary.cpp
)

#### Function ####

SET(LBCPP_FUNCTION_SOURCES
  ../include/lbcpp/Function/predeclarations.h
  ../include/lbcpp/Function/Function.h
  Function/Function.cpp  
  ../include/lbcpp/Function/ScalarFunction.h
  ../include/lbcpp/Function/ScalarObjectFunction.h
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
  Function/MiscFunctions/IdentityFunction.h
  Function/MiscFunctions/ComposeFunction.h
  Function/MiscFunctions/MultiplyFunction.h
  Function/MiscFunctions/LoadFromFileFunction.h
  Function/MiscFunctions/SetFieldFunction.h
  Function/MiscFunctions/SelectVariableFunctions.h
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
  Function/ScalarObjectFunction/BinarySumScalarObjectFunction.h
  Function/ScalarObjectFunction/MultiplyByScalarObjectFunction.h
  Function/ScalarObjectFunction/ScalarObjectFunctionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ScalarObjectFunctionLibrary.cpp
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

#### Operator ####

SET(LBCPP_OPERATOR_SOURCES
  ../include/lbcpp/Operator/Operator.h
  ../include/lbcpp/Operator/FeatureGenerator.h
  Operator/FeatureGenerator.cpp
  Operator/Operator.cpp
  Operator/OperatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/OperatorLibrary.cpp
)

SET(LBCPP_OPERATOR_CONTAINER_SOURCES
  Operator/Container/AccumulateContainerOperator.h
  Operator/Container/DiscretizeContainerOperator.h
  Operator/Container/SegmentContainerOperator.h
  Operator/Container/ApplyOnContainerOperator.h
  Operator/Container/ContainerOperatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerOperatorLibrary.cpp
)

SET(LBCPP_OPERATOR_FEATURE_GENERATOR_SOURCES
  Operator/FeatureGenerator/ConcatenateFeatureGenerator.h
  Operator/FeatureGenerator/WindowFeatureGenerator.h
  Operator/FeatureGenerator/EnumerationFeatureGenerator.h
  Operator/FeatureGenerator/EnumerationDistributionFeatureGenerator.h
  Operator/FeatureGenerator/FeatureGeneratorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FeatureGeneratorLibrary.cpp
)

#### Perception ####

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

#### Inference ####

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
  NumericalLearning/NumericalLearningLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningLibrary.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_SOURCES
  NumericalLearning/LossFunction/HingeLossFunction.h
  NumericalLearning/LossFunction/LogBinomialLossFunction.h
  NumericalLearning/LossFunction/OneAgainstAllMultiClassLossFunction.h
  NumericalLearning/LossFunction/MostViolatedMultiClassLossFunction.h
  NumericalLearning/LossFunction/LogBinomialMultiClassLossFunction.h
  NumericalLearning/LossFunction/AdditiveRankingLossFunction.h
  NumericalLearning/LossFunction/AllPairsRankingLossFunction.h
  NumericalLearning/LossFunction/MostViolatedPairRankingLossFunction.h
  NumericalLearning/LossFunction/BestAgainstAllRankingLossFunction.h
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
  DecisionTree/DecisionTreeLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeLibrary.cpp
)

SET(LBCPP_DECISION_TREE_INFERENCE_SOURCES
  DecisionTree/Inference/BinaryDecisionTree.h
  DecisionTree/Inference/BinaryDecisionTree.cpp
  DecisionTree/Inference/ExtraTreeInference.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.cpp
  DecisionTree/Inference/DecisionTreeExampleVector.h
  DecisionTree/Inference/DecisionTreeExampleVector.cpp
  DecisionTree/Inference/SplitScoringFunction.h
  DecisionTree/Inference/SplitScoringFunction.cpp
  DecisionTree/Inference/BinaryDecisionTreeSplitter.h
  DecisionTree/Inference/BinaryDecisionTreeSplitter.cpp
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

  # Data
  ${LBCPP_DATA_SOURCES}
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
  
  # Network
  ${LBCPP_NETWORK_SOURCES}
  
  # Distribution
  ${LBCPP_PROBABILITY_DISTRIBUTION_SOURCES}
    ${LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_SOURCES}

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

  # Operator
  ${LBCPP_OPERATOR_SOURCES}
    ${LBCPP_OPERATOR_CONTAINER_SOURCES}
    ${LBCPP_OPERATOR_FEATURE_GENERATOR_SOURCES}

  # Perception
  ${LBCPP_PERCEPTION_SOURCES}
    ${LBCPP_PERCEPTION_SIMPLE_SOURCES}
    ${LBCPP_PERCEPTION_MODIFIER_SOURCES}
    ${LBCPP_PERCEPTION_CONTAINER_SOURCES}

  # Optimizer
  ${LBCPP_OPTIMIZER_SOURCES}
    ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES}
    ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES}

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
  
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
)

SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Core
SOURCE_GROUP(Core FILES ${LBCPP_CORE_SOURCES})
SOURCE_SUB_GROUP(Core impl FILES ${LBCPP_CORE_IMPL_HEADERS})
SOURCE_SUB_GROUP(Core Type FILES ${LBCPP_CORE_TYPE_SOURCES})
SOURCE_SUB_GROUP(Core Object FILES ${LBCPP_CORE_OBJECT_SOURCES})

## Data
SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})

## Network
SOURCE_GROUP(Network FILES ${LBCPP_NETWORK_SOURCES})

## Distribution
SOURCE_GROUP(Distribution FILES ${LBCPP_PROBABILITY_DISTRIBUTION_SOURCES})
SOURCE_SUB_GROUP(Distribution Builder FILES ${LBCPP_PROBABILITY_DISTRIBUTION_BUILDER_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${LBCPP_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${LBCPP_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${LBCPP_EXECUTION_CONTEXT_SOURCES})

## Function
SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function MiscFunctions FILES ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES})
SOURCE_SUB_GROUP(Function ScalarFunction FILES ${LBCPP_FUNCTION_SCALAR_SOURCES})
SOURCE_SUB_GROUP(Function ScalarObjectFunction FILES ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Function IterationFunction FILES ${LBCPP_FUNCTION_ITERATION_SOURCES})
SOURCE_SUB_GROUP(Function StoppingCriterion FILES ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES})
SOURCE_SUB_GROUP(Function Predicate FILES ${LBCPP_FUNCTION_PREDICATE_SOURCES})
SOURCE_SUB_GROUP(Function Evaluator FILES ${LBCPP_FUNCTION_EVALUATOR_SOURCES})

## Operator
SOURCE_GROUP(Operator FILES ${LBCPP_OPERATOR_SOURCES})
SOURCE_SUB_GROUP(Operator Container FILES ${LBCPP_OPERATOR_CONTAINER_SOURCES})
SOURCE_SUB_GROUP(Operator FeatureGenerator FILES ${LBCPP_OPERATOR_FEATURE_GENERATOR_SOURCES})

## Perception
SOURCE_GROUP(Perception FILES ${LBCPP_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP(Perception Simple FILES ${LBCPP_PERCEPTION_SIMPLE_SOURCES})
SOURCE_SUB_GROUP(Perception Modifier FILES ${LBCPP_PERCEPTION_MODIFIER_SOURCES})
SOURCE_SUB_GROUP(Perception Container FILES ${LBCPP_PERCEPTION_CONTAINER_SOURCES})

## Optimizer
SOURCE_GROUP(Optimizer FILES ${LBCPP_OPTIMIZER_SOURCES})
SOURCE_SUB_GROUP(Optimizer ObjectiveFunction FILES ${LBCPP_OPTIMIZER_OBJECTIVE_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Optimizer Optimizer FILES ${LBCPP_OPTIMIZER_OPTIMIZER_SOURCES})

## Inference
SOURCE_GROUP(Inference FILES ${LBCPP_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Inference Callback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Inference ReductionInference FILES ${LBCPP_INFERENCE_REDUCTION_SOURCES})
SOURCE_SUB_GROUP(Inference MetaInference FILES ${LBCPP_INFERENCE_META_SOURCES})
SOURCE_SUB_GROUP(Inference BatchLearner FILES ${LBCPP_INFERENCE_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Inference OnlineLearner FILES ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES})

## NumericalLearning
SOURCE_GROUP(NumericalLearning FILES ${LBCPP_NUMERICAL_LEARNING_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning Math FILES ${LBCPP_NUMERICAL_LEARNING_MATH_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning LossFunction FILES ${LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning Perception FILES ${LBCPP_NUMERICAL_LEARNING_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning Inference FILES ${LBCPP_NUMERICAL_LEARNING_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning BatchLearner FILES ${LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP(NumericalLearning OnlineLearner FILES ${LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_SOURCES})

## DecisionTree
SOURCE_GROUP(DecisionTree FILES ${LBCPP_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP(DecisionTree Inference FILES ${LBCPP_DECISION_TREE_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(DecisionTree RTree FILES ${LBCPP_DECISION_TREE_RTREE_SOURCES})

## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-core SHARED ${LBCPP_CORE})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-core ${LBCPP_CORE})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-core juce)
