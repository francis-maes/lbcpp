INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppClasses.xml LBCppClasses.cpp)

GENERATE_INTROSPECTION_CODE(Data/Type/TypeClasses.xml TypeClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Stream/StreamClasses.xml StreamClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerClasses.xml ContainerClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataClasses.xml DataClasses.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionClasses.xml FunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/MiscFunctions/MiscFunctionsClasses.xml MiscFunctionsClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionClasses.xml ScalarFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarObjectFunction/ScalarObjectFunctionClasses.xml ScalarObjectFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/IterationFunction/IterationFunctionClasses.xml IterationFunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/StoppingCriterion/StoppingCriterionClasses.xml StoppingCriterionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Predicate/PredicateClasses.xml PredicateClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Perception/PerceptionClasses.xml PerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Perception/Features/PerceptionFeaturesClasses.xml PerceptionFeaturesClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/Evaluator/EvaluatorClasses.xml EvaluatorClasses.cpp)

GENERATE_INTROSPECTION_CODE(Inference/InferenceClasses.xml InferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceCallback/InferenceCallbackClasses.xml InferenceCallbackClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceContext/InferenceContextClasses.xml InferenceContextClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ReductionInference/ReductionInferenceClasses.xml ReductionInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/MetaInference/MetaInferenceClasses.xml MetaInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/NumericalInference/NumericalInferenceClasses.xml NumericalInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/DecisionTreeInference/DecisionTreeInferenceClasses.xml DecisionTreeInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceOnlineLearner/InferenceOnlineLearnerClasses.xml InferenceOnlineLearnerClasses.cpp)

##### Data ####

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
  ../include/lbcpp/Data/ProbabilityDistribution.h
  ../include/lbcpp/Data/RandomGenerator.h
  Data/RandomGenerator.cpp  
  Data/ProbabilityDistribution.cpp
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
  Data/Type/TupleType.h
  Data/Type/TypeClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/TypeClasses.cpp
)

SET(LBCPP_DATA_STREAM_SOURCES
  Data/Stream/ApplyFunctionStream.h
  Data/Stream/DirectoryFileStream.h
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

##### FUNCTION ####

SET(LBCPP_FUNCTION_SOURCES
  ../include/lbcpp/Function/predeclarations.h
  ../include/lbcpp/Function/Function.h
  ../include/lbcpp/Function/ScalarFunction.h
  ../include/lbcpp/Function/ScalarObjectFunction.h
  Function/ScalarFunction.cpp
  ../include/lbcpp/Function/IterationFunction.h
  ../include/lbcpp/Function/StoppingCriterion.h
  ../include/lbcpp/Function/Predicate.h
  ../include/lbcpp/Function/Perception.h
  Function/Perception.cpp
  ../include/lbcpp/Function/PerceptionMaths.h
  Function/PerceptionMaths.cpp
  ../include/lbcpp/Function/PerceptionRewriter.h
  Function/PerceptionRewriter.cpp
  ../include/lbcpp/Function/Evaluator.h  
  Function/Evaluator.cpp
  Function/FunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FunctionClasses.cpp
)

SET(LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES
  Function/MiscFunctions/LoadFromFileFunction.h
  Function/MiscFunctions/SetFieldFunction.h
  Function/MiscFunctions/SelectPairFieldsFunction.h
  Function/MiscFunctions/WindowToIndicesFunction.h
  Function/MiscFunctions/VariableToIndicesFunction.h
  Function/MiscFunctions/MiscFunctionsClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MiscFunctionsClasses.cpp
)

SET(LBCPP_FUNCTION_SCALAR_SOURCES
  Function/ScalarFunction/ComposeScalarFunction.h
  Function/ScalarFunction/MultiplyByScalarFunction.h
  Function/ScalarFunction/HingeLossFunction.h
  Function/ScalarFunction/LogBinomialLossFunction.h
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
  Function/ScalarObjectFunction/SumOfSquaresScalarObjectFunction.h
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

SET(LBCPP_FUNCTION_PERCEPTION_SOURCES
  Function/Perception/NullPerception.h
  Function/Perception/IdentityPerception.h
  Function/Perception/RewritedPerception.h
  Function/Perception/ProductPerception.h
  Function/Perception/ProductPerception.cpp
  Function/Perception/WindowPerception.h
  Function/Perception/HistogramPerception.h
  Function/Perception/HistogramPerception.cpp
  Function/Perception/BoundsProximityPerception.h
  Function/Perception/ProbabilityDistributionPerception.h
  Function/Perception/BiContainerPerception.h
  Function/Perception/BiVariablePerception.h
  Function/Perception/PreprocessPerception.h
  Function/Perception/FlattenPerception.h
  Function/Perception/CollapsePerception.h
  Function/Perception/FunctionBasedPerception.h
  Function/Perception/PerceptionRewriterRules.h
  Function/Perception/PerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PerceptionClasses.cpp
)

SET(LBCPP_FUNCTION_PERCEPTION_FEATURES_SOURCES
  Function/Perception/Features/BooleanFeatures.h
  Function/Perception/Features/EnumValueFeatures.h
  Function/Perception/Features/DiscretizedNumberFeatures.h
  Function/Perception/Features/HardDiscretizedNumberFeatures.h
  Function/Perception/Features/SoftDiscretizedNumberFeatures.h
  Function/Perception/Features/SignedNumberFeatures.h
  Function/Perception/Features/BiVariableFeatures.h
  Function/Perception/Features/PerceptionFeaturesClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PerceptionFeaturesClasses.cpp
)

SET(LBCPP_FUNCTION_EVALUATOR_SOURCES
  Function/Evaluator/ContainerElementsEvaluator.h
  Function/Evaluator/BinaryClassificationConfusionEvaluator.h
  Function/Evaluator/ClassificationAccuracyEvaluator.h
  Function/Evaluator/RegressionErrorEvaluator.h
  Function/Evaluator/EvaluatorClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/EvaluatorClasses.cpp
)


##### INFERENCE ####

SET(LBCPP_INFERENCE_SOURCES
  ../include/lbcpp/Inference/predeclarations.h
  ../include/lbcpp/Inference/Inference.h
  ../include/lbcpp/Inference/DecoratorInference.h
  ../include/lbcpp/Inference/SequentialInference.h
  ../include/lbcpp/Inference/ParallelInference.h
  Inference/Inference.cpp
  ../include/lbcpp/Inference/InferenceStack.h
  ../include/lbcpp/Inference/InferenceContext.h
  Inference/InferenceContext.cpp
  ../include/lbcpp/Inference/InferenceCallback.h
  ../include/lbcpp/Inference/InferenceResultCache.h
  Inference/InferenceCallback.cpp
  ../include/lbcpp/Inference/InferenceOnlineLearner.h
  Inference/InferenceOnlineLearner.cpp
  Inference/InferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceClasses.cpp
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  Inference/InferenceCallback/CancelAfterStepCallback.h
  Inference/InferenceCallback/CacheInferenceCallback.h
  Inference/InferenceCallback/OnlineLearningInferenceCallback.h
  Inference/InferenceCallback/EvaluationInferenceCallback.h
  Inference/InferenceCallback/InferenceCallbackClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceCallbackClasses.cpp  
)

SET(LBCPP_INFERENCE_CONTEXT_SOURCES
  Inference/InferenceContext/SingleThreadedInferenceContext.h
  Inference/InferenceContext/InferenceThreadPoolJob.h
  Inference/InferenceContext/ThreadOwnedInferenceContext.h
  Inference/InferenceContext/MultiThreadedInferenceContext.h
  Inference/InferenceContext/InferenceContextClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceContextClasses.cpp
)

SET(LBCPP_INFERENCE_NUMERICAL_SOURCES
  Inference/NumericalInference/NumericalInference.h
  Inference/NumericalInference/NumericalInference.cpp
  Inference/NumericalInference/LinearInference.h
  Inference/NumericalInference/BinaryClassificationInference.h
  Inference/NumericalInference/TransferFunctionDecoratorInference.h
  Inference/NumericalInference/RegressionInference.h
  Inference/NumericalInference/NumericalInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NumericalInferenceClasses.cpp  
)

SET(LBCPP_INFERENCE_DECISION_TREE_SOURCES
  Inference/DecisionTreeInference/BinaryDecisionTree.h
  Inference/DecisionTreeInference/BinaryDecisionTree.cpp
  Inference/DecisionTreeInference/ExtraTreeInference.h
  Inference/DecisionTreeInference/ExtraTreeInferenceLearner.h
  Inference/DecisionTreeInference/ExtraTreeInferenceLearner.cpp
  Inference/DecisionTreeInference/DecisionTreeInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/DecisionTreeInferenceClasses.cpp  
)

SET(LBCPP_INFERENCE_REDUCTION_SOURCES
  Inference/ReductionInference/OneAgainstAllClassificationInference.h
  Inference/ReductionInference/ParallelVoteInference.h
  Inference/ReductionInference/SharedParallelVectorInference.h
  Inference/ReductionInference/ReductionInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ReductionInferenceClasses.cpp
)

SET(LBCPP_INFERENCE_META_SOURCES
  Inference/MetaInference/CallbackBasedDecoratorInference.h
  Inference/MetaInference/RunOnSupervisedExamplesInference.h
  Inference/MetaInference/CrossValidationInference.h
  Inference/MetaInference/StaticSequentialInferenceLearner.h
  Inference/MetaInference/StaticParallelInferenceLearner.h
  Inference/MetaInference/SharedParallelInferenceLearner.h
  Inference/MetaInference/DecoratorInferenceLearner.h
  Inference/MetaInference/OnlineToBatchInferenceLearner.h
  Inference/MetaInference/MetaInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MetaInferenceClasses.cpp
)

SET(LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES
  Inference/InferenceOnlineLearner/GradientDescentOnlineLearner.h
  Inference/InferenceOnlineLearner/GradientDescentOnlineLearner.cpp
  Inference/InferenceOnlineLearner/StochasticGradientDescentOnlineLearner.h
  Inference/InferenceOnlineLearner/BatchGradientDescentOnlineLearner.h
  Inference/InferenceOnlineLearner/RandomizerInferenceOnlineLearner.h
  Inference/InferenceOnlineLearner/StoppingCriterionInferenceOnlineLearner.h
  Inference/InferenceOnlineLearner/InferenceOnlineLearnerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceOnlineLearnerClasses.cpp
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
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
    ${LBCPP_DATA_IMPL_HEADERS}
  
  # Function
  ${LBCPP_FUNCTION_SOURCES}
    ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES}
    ${LBCPP_FUNCTION_ITERATION_SOURCES}
    ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES}
    ${LBCPP_FUNCTION_PREDICATE_SOURCES}
    ${LBCPP_FUNCTION_PERCEPTION_SOURCES}
      ${LBCPP_FUNCTION_PERCEPTION_FEATURES_SOURCES}
    ${LBCPP_FUNCTION_EVALUATOR_SOURCES}

  # Inference
  ${LBCPP_INFERENCE_SOURCES}
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
  ${LBCPP_INFERENCE_CONTEXT_SOURCES}
  ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES}
  ${LBCPP_INFERENCE_INFERENCE_SOURCES}
    ${LBCPP_INFERENCE_NUMERICAL_SOURCES}
    ${LBCPP_INFERENCE_DECISION_TREE_SOURCES}
    ${LBCPP_INFERENCE_REDUCTION_SOURCES}
    ${LBCPP_INFERENCE_META_SOURCES}
)

SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Data

SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data impl FILES ${LBCPP_DATA_IMPL_HEADERS})
SOURCE_SUB_GROUP(Data Type FILES ${LBCPP_DATA_TYPE_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})

## Function

SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function MiscFunctions FILES ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES})
SOURCE_SUB_GROUP(Function ScalarFunction FILES ${LBCPP_FUNCTION_SCALAR_SOURCES})
SOURCE_SUB_GROUP(Function ScalarObjectFunction FILES ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Function IterationFunction FILES ${LBCPP_FUNCTION_ITERATION_SOURCES})
SOURCE_SUB_GROUP(Function StoppingCriterion FILES ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES})
SOURCE_SUB_GROUP(Function Predicate FILES ${LBCPP_FUNCTION_PREDICATE_SOURCES})
SOURCE_SUB_GROUP(Function Perception FILES ${LBCPP_FUNCTION_PERCEPTION_SOURCES})
SOURCE_SUB_GROUP2(Function Perception Features FILES ${LBCPP_FUNCTION_PERCEPTION_FEATURES_SOURCES})
SOURCE_SUB_GROUP(Function Evaluator FILES ${LBCPP_FUNCTION_EVALUATOR_SOURCES})

## Inference

SOURCE_GROUP(Inference FILES ${LBCPP_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Inference Callback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Inference Context FILES ${LBCPP_INFERENCE_CONTEXT_SOURCES})
SOURCE_SUB_GROUP(Inference OnlineLearner FILES ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES})

SOURCE_SUB_GROUP(Inference NumericalInference FILES ${LBCPP_INFERENCE_NUMERICAL_SOURCES})
SOURCE_SUB_GROUP(Inference DecisionTreeInference FILES ${LBCPP_INFERENCE_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP(Inference ReductionInference FILES ${LBCPP_INFERENCE_REDUCTION_SOURCES})
SOURCE_SUB_GROUP(Inference MetaInference FILES ${LBCPP_INFERENCE_META_SOURCES})


ADD_LIBRARY(lbcpp-core ${LBCPP_CORE})

TARGET_LINK_LIBRARIES(lbcpp-core juce)
