INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppClasses.xml LBCppClasses.cpp)

GENERATE_INTROSPECTION_CODE(Data/Type/TypeClasses.xml TypeClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Stream/StreamClasses.xml StreamClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerClasses.xml ContainerClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataClasses.xml DataClasses.cpp)

GENERATE_INTROSPECTION_CODE(ProbabilityDistribution/ProbabilityDistributionClasses.xml ProbabilityDistributionClasses.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionClasses.xml ExecutionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackClasses.xml ExecutionCallbackClasses.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextClasses.xml ExecutionContextClasses.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionClasses.xml FunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/MiscFunctions/MiscFunctionsClasses.xml MiscFunctionsClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionClasses.xml ScalarFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarObjectFunction/ScalarObjectFunctionClasses.xml ScalarObjectFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/IterationFunction/IterationFunctionClasses.xml IterationFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/StoppingCriterion/StoppingCriterionClasses.xml StoppingCriterionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Predicate/PredicateClasses.xml PredicateClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Evaluator/EvaluatorClasses.xml EvaluatorClasses.cpp)

GENERATE_INTROSPECTION_CODE(Perception/PerceptionClasses.xml PerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Simple/SimplePerceptionClasses.xml SimplePerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Modifier/ModifierPerceptionClasses.xml ModifierPerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Perception/Container/ContainerPerceptionClasses.xml ContainerPerceptionClasses.cpp)

GENERATE_INTROSPECTION_CODE(Inference/InferenceClasses.xml InferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceCallback/InferenceCallbackClasses.xml InferenceCallbackClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ReductionInference/ReductionInferenceClasses.xml ReductionInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/MetaInference/MetaInferenceClasses.xml MetaInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/BatchLearner/InferenceBatchLearnerClasses.xml InferenceBatchLearnerClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/OnlineLearner/InferenceOnlineLearnerClasses.xml InferenceOnlineLearnerClasses.cpp)

GENERATE_INTROSPECTION_CODE(NumericalLearning/NumericalLearningClasses.xml NumericalLearningClasses.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/LossFunction/NumericalLearningLossFunctionClasses.xml NumericalLearningLossFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Perception/NumericalLearningPerceptionClasses.xml NumericalLearningPerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/Inference/NumericalLearningInferenceClasses.xml NumericalLearningInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/BatchLearner/NumericalLearningBatchLearnerClasses.xml NumericalLearningBatchLearnerClasses.cpp)
GENERATE_INTROSPECTION_CODE(NumericalLearning/OnlineLearner/NumericalLearningOnlineLearnerClasses.xml NumericalLearningOnlineLearnerClasses.cpp)

GENERATE_INTROSPECTION_CODE(DecisionTree/DecisionTreeClasses.xml DecisionTreeClasses.cpp)
GENERATE_INTROSPECTION_CODE(DecisionTree/Inference/DecisionTreeInferenceClasses.xml DecisionTreeInferenceClasses.cpp)

#### Data ####

SET(LBCPP_DATA_SOURCES
  ../include/lbcpp/Data/Utilities.h  
  Data/Utilities.cpp
  ../include/lbcpp/Data/ReferenceCountedObject.h
  ../include/lbcpp/Data/predeclarations.h
  ../include/lbcpp/Data/Object.h
  Data/Object.cpp
  ../include/lbcpp/Data/Type.h
  Data/Type.cpp
  Data/Enumeration.cpp
  Data/Class.cpp
  ../include/lbcpp/Data/TemplateType.h
  Data/TemplateType.cpp
  ../include/lbcpp/Data/Variable.h
  Data/Variable.cpp
  ../include/lbcpp/Data/Pair.h
  ../include/lbcpp/Data/XmlSerialisation.h  
  Data/XmlSerialisation.cpp
  ../include/lbcpp/Data/DynamicObject.h
  Data/DynamicObject.cpp
  ../include/lbcpp/Data/Stream.h
  Data/Stream.cpp
  ../include/lbcpp/Data/Container.h
  Data/Container.cpp
  ../include/lbcpp/Data/Vector.h
  Data/Vector.cpp
  ../include/lbcpp/Data/SymmetricMatrix.h
  Data/SymmetricMatrix.cpp
  ../include/lbcpp/Data/RandomGenerator.h
  Data/RandomGenerator.cpp  
  ../include/lbcpp/Data/RandomVariable.h  
  ../include/lbcpp/Data/Consumer.h
  Data/Consumer.cpp
  ../include/lbcpp/Data/Cache.h
  Data/Cache.cpp
  Data/DataClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DataClasses.cpp
)  

SET(LBCPP_DATA_IMPL_HEADERS
  ../include/lbcpp/Data/impl/VariableValue.hpp
  ../include/lbcpp/Data/impl/Variable.hpp
)

SET(LBCPP_DATA_TYPE_SOURCES
  Data/Type/TopLevelType.h
  Data/Type/NilType.h
  Data/Type/BuiltinType.h
  Data/Type/BooleanType.h
  Data/Type/IntegerType.h
  Data/Type/DoubleType.h
  Data/Type/StringType.h
  Data/Type/FileType.h
  Data/Type/TypeClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/TypeClasses.cpp
)

SET(LBCPP_DATA_OBJECT_SOURCES
  Data/Object/SparseGenericObject.h
  Data/Object/SparseDoubleObject.h
  Data/Object/DenseGenericObject.h
  Data/Object/DenseDoubleObject.h
  Data/Object/DenseObjectObject.h
)

SET(LBCPP_DATA_STREAM_SOURCES
  Data/Stream/ApplyFunctionStream.h
  Data/Stream/DirectoryFileStream.h
  Data/Stream/DirectoryPairFileStream.h
  Data/Stream/StreamClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/StreamClasses.cpp
)

SET(LBCPP_DATA_CONTAINER_SOURCES
  Data/Container/ApplyFunctionContainer.h
  Data/Container/SubsetContainer.h
  Data/Container/DuplicatedContainer.h
  Data/Container/RangeContainer.h
  Data/Container/ExcludeRangeContainer.h
  Data/Container/ContainerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerClasses.cpp
)

#### ProbabilityDistribution ####

SET(LBCPP_PROBABILITY_DISTRIBUTION_SOURCES
  ../include/lbcpp/ProbabilityDistribution/ProbabilityDistribution.h
  ../include/lbcpp/ProbabilityDistribution/DiscreteProbabilityDistribution.h
  ProbabilityDistribution/DiscreteProbabilityDistribution.cpp
  ../include/lbcpp/ProbabilityDistribution/ContinuousProbabilityDistribution.h
  ProbabilityDistribution/ContinuousProbabilityDistribution.cpp
  ProbabilityDistribution/ProbabilityDistributionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProbabilityDistributionClasses.cpp
)

#### Execution ####

SET(LBCPP_EXECUTION_SOURCES
  ../include/lbcpp/Execution/predeclarations.h
  ../include/lbcpp/Execution/WorkUnit.h
  Execution/WorkUnit.cpp
  ../include/lbcpp/Execution/ExecutionStack.h
  ../include/lbcpp/Execution/ExecutionCallback.h
  ../include/lbcpp/Execution/ExecutionContext.h
  Execution/ExecutionContext.cpp
  Execution/ExecutionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionClasses.cpp
)

SET(LBCPP_EXECUTION_CALLBACK_SOURCES
  Execution/Callback/SilentExecutionCallback.h
  Execution/Callback/ConsoleExecutionCallback.h
  Execution/Callback/ExecutionCallbackClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackClasses.cpp
)

SET(LBCPP_EXECUTION_CONTEXT_SOURCES
  Execution/Context/SingleThreadedExecutionContext.h
  Execution/Context/DecoratorExecutionContext.h
  Execution/Context/MultiThreadedExecutionContext.h
  Execution/Context/ExecutionContextClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionContextClasses.cpp
)

#### Function ####

SET(LBCPP_FUNCTION_SOURCES
  ../include/lbcpp/Function/predeclarations.h
  ../include/lbcpp/Function/Function.h
  ../include/lbcpp/Function/ScalarFunction.h
  ../include/lbcpp/Function/ScalarObjectFunction.h
  Function/ScalarFunction.cpp
  ../include/lbcpp/Function/IterationFunction.h
  ../include/lbcpp/Function/StoppingCriterion.h
  ../include/lbcpp/Function/Predicate.h
  ../include/lbcpp/Function/ObjectiveFunction.h
  ../include/lbcpp/Function/Evaluator.h  
  Function/Evaluator.cpp
  Function/FunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FunctionClasses.cpp
)

SET(LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES
  Function/MiscFunctions/LoadFromFileFunction.h
  Function/MiscFunctions/SetFieldFunction.h
  Function/MiscFunctions/SelectVariableFunctions.h
  Function/MiscFunctions/MiscFunctionsClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MiscFunctionsClasses.cpp
)

SET(LBCPP_FUNCTION_SCALAR_SOURCES
  Function/ScalarFunction/ComposeScalarFunction.h
  Function/ScalarFunction/MultiplyByScalarFunction.h
  Function/ScalarFunction/AbsFunction.h
  Function/ScalarFunction/SquareFunction.h
  Function/ScalarFunction/AddConstantScalarFunction.h
  Function/ScalarFunction/AngleDifferenceFunction.h
  Function/ScalarFunction/ScalarFunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ScalarFunctionClasses.cpp
)

SET(LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES
  Function/ScalarObjectFunction/BinarySumScalarObjectFunction.h
  Function/ScalarObjectFunction/MultiplyByScalarObjectFunction.h
  Function/ScalarObjectFunction/ScalarObjectFunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ScalarObjectFunctionClasses.cpp
)

SET(LBCPP_FUNCTION_ITERATION_SOURCES
  Function/IterationFunction/ConstantIterationFunction.h
  Function/IterationFunction/InvLinearIterationFunction.h
  Function/IterationFunction/IterationFunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/IterationFunctionClasses.cpp
)

SET(LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES
  Function/StoppingCriterion/MaxIterationsStoppingCriterion.h
  Function/StoppingCriterion/MaxIterationsWithoutImprovementStoppingCriterion.h
  Function/StoppingCriterion/AverageImprovementStoppingCriterion.h
  Function/StoppingCriterion/LogicalOrStoppingCriterion.h
  Function/StoppingCriterion/StoppingCriterionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/StoppingCriterionClasses.cpp
)

SET(LBCPP_FUNCTION_PREDICATE_SOURCES
  Function/Predicate/BinaryPredicate.h
  Function/Predicate/UnaryComparisonPredicate.h
  Function/Predicate/PredicateClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PredicateClasses.cpp
)

SET(LBCPP_FUNCTION_EVALUATOR_SOURCES
  Function/Evaluator/ContainerElementsEvaluator.h
  Function/Evaluator/BinaryClassificationConfusionEvaluator.h
  Function/Evaluator/ClassificationAccuracyEvaluator.h
  Function/Evaluator/RegressionErrorEvaluator.h
  Function/Evaluator/SaveToDirectoryEvaluator.h
  Function/Evaluator/EvaluatorClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/EvaluatorClasses.cpp
)

#### Perception ####

SET(LBCPP_PERCEPTION_SOURCES
  ../include/lbcpp/Perception/Perception.h
  Perception/Perception.cpp
  ../include/lbcpp/Perception/PerceptionRewriter.h
  Perception/PerceptionRewriter.cpp
  Perception/PerceptionRewriterRules.h
  Perception/PerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PerceptionClasses.cpp
)

SET(LBCPP_PERCEPTION_SIMPLE_SOURCES
  Perception/Simple/NullPerception.h
  Perception/Simple/IdentityPerception.h
  Perception/Simple/FunctionBasedPerception.h
  Perception/Simple/SimplePerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/SimplePerceptionClasses.cpp
)

SET(LBCPP_PERCEPTION_MODIFIER_SOURCES
  Perception/Modifier/RewritedPerception.h
  Perception/Modifier/ProductPerception.h
  Perception/Modifier/ProductPerception.cpp
  Perception/Modifier/ComposePerception.h
  Perception/Modifier/FlattenPerception.h
  Perception/Modifier/CollapsePerception.h
  Perception/Modifier/SelectAndMakeProductsPerception.h
  Perception/Modifier/ModifierPerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ModifierPerceptionClasses.cpp
)

SET(LBCPP_PERCEPTION_CONTAINER_SOURCES
  Perception/Container/WindowPerception.h
  Perception/Container/HistogramPerception.h
  Perception/Container/HistogramPerception.cpp
  Perception/Container/SegmentsPerception.h
  Perception/Container/SegmentsPerception.cpp
  Perception/Container/BoundsProximityPerception.h
  Perception/Container/ProbabilityDistributionPerception.h
  Perception/Container/InputLabelPairPerception.h
  Perception/Container/ContainerPerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ContainerPerceptionClasses.cpp
)

#### Inference ####

SET(LBCPP_INFERENCE_SOURCES
  ../include/lbcpp/Inference/predeclarations.h
  ../include/lbcpp/Inference/Inference.h
  ../include/lbcpp/Inference/DecoratorInference.h
  ../include/lbcpp/Inference/SequentialInference.h
  ../include/lbcpp/Inference/ParallelInference.h
  Inference/Inference.cpp
  ../include/lbcpp/Inference/InferenceContext.h
  Inference/InferenceContext.cpp
  ../include/lbcpp/Inference/InferenceCallback.h
  ../include/lbcpp/Inference/InferenceResultCache.h
  Inference/InferenceCallback.cpp
  ../include/lbcpp/Inference/InferenceBatchLearner.h
  Inference/InferenceBatchLearner.cpp
  ../include/lbcpp/Inference/InferenceOnlineLearner.h
  Inference/InferenceOnlineLearner.cpp
  Inference/InferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceClasses.cpp
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  Inference/InferenceCallback/CancelAfterStepCallback.h
  Inference/InferenceCallback/CacheInferenceCallback.h
  Inference/InferenceCallback/EvaluationInferenceCallback.h
  Inference/InferenceCallback/InferenceCallbackClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceCallbackClasses.cpp  
)

SET(LBCPP_INFERENCE_REDUCTION_SOURCES
  Inference/ReductionInference/OneAgainstAllClassificationInference.h
  Inference/ReductionInference/OneAgainstOneClassificationInference.h
  Inference/ReductionInference/RankingBasedClassificationInference.h
  Inference/ReductionInference/ParallelVoteInference.h
  Inference/ReductionInference/SharedParallelVectorInference.h
  Inference/ReductionInference/ReductionInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ReductionInferenceClasses.cpp
)

SET(LBCPP_INFERENCE_META_SOURCES
  Inference/MetaInference/CallbackBasedDecoratorInference.h
  Inference/MetaInference/RunOnSupervisedExamplesInference.h
  Inference/MetaInference/CrossValidationInference.h
  Inference/MetaInference/MetaInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MetaInferenceClasses.cpp
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
  Inference/BatchLearner/InferenceBatchLearnerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceBatchLearnerClasses.cpp
)

SET(LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES
  Inference/OnlineLearner/ComputeScoreOnlineLearner.h
  Inference/OnlineLearner/ComputeEvaluatorOnlineLearner.h
  Inference/OnlineLearner/RandomizerOnlineLearner.h
  Inference/OnlineLearner/StoppingCriterionOnlineLearner.h
  Inference/OnlineLearner/SaveScoresToGnuPlotFileOnlineLearner.h
  Inference/OnlineLearner/InferenceOnlineLearnerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceOnlineLearnerClasses.cpp
)

#### NumericalLearning ####

SET(LBCPP_NUMERICAL_LEARNING_SOURCES
  ../include/lbcpp/NumericalLearning/LossFunctions.h
  NumericalLearning/LossFunctions.cpp
  ../include/lbcpp/NumericalLearning/NumericalInference.h
  ../include/lbcpp/NumericalLearning/NumericalSupervisedInference.h
  ../include/lbcpp/NumericalLearning/NumericalLearning.h
  NumericalLearning/NumericalLearning.cpp
  NumericalLearning/NumericalLearningClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningClasses.cpp
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
  NumericalLearning/LossFunction/NumericalLearningLossFunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningLossFunctionClasses.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_MATH_SOURCES
  NumericalLearning/Math/DoubleConstUnaryOperation.h
  NumericalLearning/Math/DoubleUnaryOperation.h
  NumericalLearning/Math/DoubleDotProductOperation.h
  NumericalLearning/Math/DoubleAssignmentOperation.h
)

SET(LBCPP_NUMERICAL_LEARNING_PERCEPTION_SOURCES
  NumericalLearning/Perception/BooleanFeatures.h
  NumericalLearning/Perception/EnumValueFeatures.h
  NumericalLearning/Perception/DiscretizedNumberFeatures.h
  NumericalLearning/Perception/HardDiscretizedNumberFeatures.h
  NumericalLearning/Perception/SoftDiscretizedNumberFeatures.h
  NumericalLearning/Perception/SignedNumberFeatures.h
  NumericalLearning/Perception/NumericalLearningPerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningPerceptionClasses.cpp
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
  NumericalLearning/Inference/NumericalLearningInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningInferenceClasses.cpp  
)

SET(LBCPP_NUMERICAL_LEARNING_BATCH_LEARNER_SOURCES
  NumericalLearning/BatchLearner/PrecomputePerceptionsNumericalInferenceLearner.h
  NumericalLearning/BatchLearner/AddBiasInferenceLearner.h
  NumericalLearning/BatchLearner/NumericalLearningBatchLearnerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningBatchLearnerClasses.cpp
)

SET(LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_SOURCES
  NumericalLearning/OnlineLearner/GradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/GradientDescentOnlineLearner.cpp
  NumericalLearning/OnlineLearner/StochasticGradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/BatchGradientDescentOnlineLearner.h
  NumericalLearning/OnlineLearner/GraftingOnlineLearner.h
  NumericalLearning/OnlineLearner/GraftingOnlineLearner.cpp
  NumericalLearning/OnlineLearner/NumericalLearningOnlineLearnerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalLearningOnlineLearnerClasses.cpp
)

#### DecisionTree ####

SET(LBCPP_DECISION_TREE_SOURCES
  ../include/lbcpp/DecisionTree/DecisionTree.h
  DecisionTree/DecisionTreeClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeClasses.cpp
)

SET(LBCPP_DECISION_TREE_INFERENCE_SOURCES
  DecisionTree/Inference/BinaryDecisionTree.h
  DecisionTree/Inference/BinaryDecisionTree.cpp
  DecisionTree/Inference/ExtraTreeInference.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.h
  DecisionTree/Inference/ExtraTreeInferenceLearner.cpp
  DecisionTree/Inference/SplitScoringFunction.h
  DecisionTree/Inference/SplitScoringFunction.cpp
  DecisionTree/Inference/BinaryDecisionTreeSplitter.h
  DecisionTree/Inference/BinaryDecisionTreeSplitter.cpp
  DecisionTree/Inference/DecisionTreeInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeInferenceClasses.cpp  
)


##### MAIN ####

SET(LBCPP_MAIN_SOURCES
  ../include/lbcpp/common.h
  ../include/lbcpp/lbcpp.h
  LBCppClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppClasses.cpp
)

SET(LBCPP_CORE
  ${LBCPP_MAIN_SOURCES}

  # Data
  ${LBCPP_DATA_SOURCES}
    ${LBCPP_DATA_TYPE_SOURCES}
    ${LBCPP_DATA_OBJECT_SOURCES}
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
    ${LBCPP_DATA_IMPL_HEADERS}
  
  # ProbabilityDistribution
  ${LBCPP_PROBABILITY_DISTRIBUTION_SOURCES}

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
)

SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Data
SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data impl FILES ${LBCPP_DATA_IMPL_HEADERS})
SOURCE_SUB_GROUP(Data Type FILES ${LBCPP_DATA_TYPE_SOURCES})
SOURCE_SUB_GROUP(Data Object FILES ${LBCPP_DATA_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})

## ProbabilityDistribution
SOURCE_GROUP(ProbabilityDistribution FILES ${LBCPP_PROBABILITY_DISTRIBUTION_SOURCES})

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

## Perception
SOURCE_GROUP(Perception FILES ${LBCPP_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP(Perception Simple FILES ${LBCPP_PERCEPTION_SIMPLE_SOURCES})
SOURCE_SUB_GROUP(Perception Modifier FILES ${LBCPP_PERCEPTION_MODIFIER_SOURCES})
SOURCE_SUB_GROUP(Perception Container FILES ${LBCPP_PERCEPTION_CONTAINER_SOURCES})

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

ADD_LIBRARY(lbcpp-core ${LBCPP_CORE})
TARGET_LINK_LIBRARIES(lbcpp-core juce)
