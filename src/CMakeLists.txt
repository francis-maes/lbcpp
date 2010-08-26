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

GENERATE_INTROSPECTION_CODE(Inference/InferenceClasses.xml InferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ReductionInference/ReductionInferenceClasses.xml ReductionInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/MetaInference/MetaInferenceClasses.xml MetaInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/NumericalInference/NumericalInferenceClasses.xml NumericalInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/DecisionTreeInference/DecisionTreeInferenceClasses.xml DecisionTreeInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/InferenceOnlineLearner/InferenceOnlineLearnerClasses.xml InferenceOnlineLearnerClasses.cpp)

##### Data ####

SET(LBCPP_DATA_SOURCES
  ../include/lbcpp/Data/ReferenceCountedObject.h
  ../include/lbcpp/Data/predeclarations.h
  ../include/lbcpp/Data/Utilities.h  
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


### old ------------------->
SET(LBCPP_OBJECT_HEADERS
  ../include/lbcpp/Object/ErrorHandler.h  
)

SET(LBCPP_UTILITIES_HEADERS
  ../include/lbcpp/Utilities/ProgressCallback.h
)

SET(LBCPP_UTILITIES_SOURCES
  Utilities.cpp
)
### old <-------------------


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
  Function/FunctionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/FunctionClasses.cpp
)

SET(LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES
  Function/MiscFunctions/LoadFromFileFunction.h
  Function/MiscFunctions/SetFieldFunction.h
  Function/MiscFunctions/SelectPairFieldsFunction.h
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
  Function/Perception/IdentityPerception.h
  Function/Perception/WindowPerception.h
  Function/Perception/PreprocessPerception.h
  Function/Perception/FlattenPerception.h
  Function/Perception/FunctionBasedPerception.h
  Function/Perception/PerceptionToFeatures.h
  Function/Perception/PerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/PerceptionClasses.cpp
)


##### INFERENCE ####

SET(LBCPP_INFERENCE_HEADERS
  ../include/lbcpp/Inference/predeclarations.h
  ../include/lbcpp/Inference/Inference.h
  ../include/lbcpp/Inference/InferenceStack.h
  ../include/lbcpp/Inference/InferenceCallback.h
  ../include/lbcpp/Inference/InferenceContext.h
  ../include/lbcpp/Inference/DecoratorInference.h
  ../include/lbcpp/Inference/SequentialInference.h
  ../include/lbcpp/Inference/ParallelInference.h
  ../include/lbcpp/Inference/InferenceOnlineLearner.h
  ../include/lbcpp/Inference/InferenceResultCache.h
  ../include/lbcpp/Inference/Evaluator.h
)

SET(LBCPP_INFERENCE_SOURCES
  Inference/Inference.cpp
  Inference/InferenceOnlineLearner.cpp
  Inference/InferenceCallback.cpp
  Inference/InferenceContext.cpp
  Inference/Evaluator.cpp
  Inference/InferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/InferenceClasses.cpp
)

SET(LBCPP_INFERENCE_NUMERICAL_SOURCES
  Inference/NumericalInference/NumericalInference.h
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
  Inference/MetaInference/StaticSequentialInferenceLearner.h
  Inference/MetaInference/StaticParallelInferenceLearner.h
  Inference/MetaInference/SharedParallelInferenceLearner.h
  Inference/MetaInference/DecoratorInferenceLearner.h
  Inference/MetaInference/OnlineToBatchInferenceLearner.h
  Inference/MetaInference/MetaInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MetaInferenceClasses.cpp
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  Inference/InferenceCallback/CancelAfterStepCallback.h
  Inference/InferenceCallback/CacheInferenceCallback.h
  Inference/InferenceCallback/OnlineLearningInferenceCallback.h
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

SET(LBCPP_INFERENCE_EVALUATOR_SOURCES
  Inference/Evaluator/ContainerElementsEvaluator.h
  Inference/Evaluator/BinaryClassificationConfusionEvaluator.h
  Inference/Evaluator/ClassificationAccuracyEvaluator.h
  Inference/Evaluator/RegressionErrorEvaluator.h
)



##### CORE ####

SET(LBCPP_CORE_HEADERS
  ../include/lbcpp/common.h
  ../include/lbcpp/lbcpp.h
)

SET(LBCPP_CORE
  ${LBCPP_CORE_HEADERS}

  # Data
  ${LBCPP_DATA_SOURCES}
    ${LBCPP_DATA_TYPE_SOURCES}
    ${LBCPP_DATA_STREAM_SOURCES}
    ${LBCPP_DATA_CONTAINER_SOURCES}
    ${LBCPP_DATA_IMPL_HEADERS}
  
  #Old -->
  ${LBCPP_OBJECT_HEADERS}
  ${LBCPP_UTILITIES_HEADERS}
  ${LBCPP_UTILITIES_SOURCES}
  ## <--

  # Function
  ${LBCPP_FUNCTION_SOURCES}
    ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES}
    ${LBCPP_FUNCTION_ITERATION_SOURCES}
    ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES}
    ${LBCPP_FUNCTION_PREDICATE_SOURCES}
    ${LBCPP_FUNCTION_PERCEPTION_SOURCES}

  # Inference
  ${LBCPP_INFERENCE_HEADERS}
  ${LBCPP_INFERENCE_SOURCES}
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
  ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES}
  ${LBCPP_INFERENCE_EVALUATOR_SOURCES}
  ${LBCPP_INFERENCE_INFERENCE_SOURCES}
    ${LBCPP_INFERENCE_NUMERICAL_SOURCES}
    ${LBCPP_INFERENCE_DECISION_TREE_SOURCES}
    ${LBCPP_INFERENCE_REDUCTION_SOURCES}
    ${LBCPP_INFERENCE_META_SOURCES}

  LBCppClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppClasses.cpp
)

SOURCE_GROUP(includes FILES ${LBCPP_CORE_HEADERS})

## Data

SOURCE_GROUP(Data FILES ${LBCPP_DATA_SOURCES})
SOURCE_SUB_GROUP(Data impl FILES ${LBCPP_DATA_IMPL_HEADERS})
SOURCE_SUB_GROUP(Data Type FILES ${LBCPP_DATA_TYPE_SOURCES})
SOURCE_SUB_GROUP(Data Stream FILES ${LBCPP_DATA_STREAM_SOURCES})
SOURCE_SUB_GROUP(Data Container FILES ${LBCPP_DATA_CONTAINER_SOURCES})

## Old -->
SOURCE_GROUP(Object FILES ${LBCPP_OBJECT_HEADERS} ${LBCPP_OBJECT_SOURCES})
SOURCE_GROUP(Utilities FILES ${LBCPP_UTILITIES_HEADERS} ${LBCPP_UTILITIES_SOURCES})
## <---

## Function

SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function MiscFunctions FILES ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES})
SOURCE_SUB_GROUP(Function ScalarFunction FILES ${LBCPP_FUNCTION_SCALAR_SOURCES})
SOURCE_SUB_GROUP(Function ScalarObjectFunction FILES ${LBCPP_FUNCTION_SCALAR_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Function IterationFunction FILES ${LBCPP_FUNCTION_ITERATION_SOURCES})
SOURCE_SUB_GROUP(Function StoppingCriterion FILES ${LBCPP_FUNCTION_STOPPING_CRITERION_SOURCES})
SOURCE_SUB_GROUP(Function Predicate FILES ${LBCPP_FUNCTION_PREDICATE_SOURCES})
SOURCE_SUB_GROUP(Function Perception FILES ${LBCPP_FUNCTION_PERCEPTION_SOURCES})

## Inference

SOURCE_GROUP(Inference FILES ${LBCPP_INFERENCE_HEADERS} ${LBCPP_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Inference NumericalInference FILES ${LBCPP_INFERENCE_NUMERICAL_SOURCES})
SOURCE_SUB_GROUP(Inference DecisionTreeInference FILES ${LBCPP_INFERENCE_DECISION_TREE_SOURCES})
SOURCE_SUB_GROUP(Inference ReductionInference FILES ${LBCPP_INFERENCE_REDUCTION_SOURCES})
SOURCE_SUB_GROUP(Inference MetaInference FILES ${LBCPP_INFERENCE_META_SOURCES})

SOURCE_SUB_GROUP(Inference Callback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Inference OnlineLearner FILES ${LBCPP_INFERENCE_ONLINE_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Inference BatchLearner FILES ${LBCPP_INFERENCE_BATCH_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Inference Evaluator FILES ${LBCPP_INFERENCE_EVALUATOR_SOURCES})

ADD_LIBRARY(lbcpp-core ${LBCPP_CORE})

TARGET_LINK_LIBRARIES(lbcpp-core juce)
