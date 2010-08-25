INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(Data/Type/TypeClasses.xml TypeClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Stream/StreamClasses.xml StreamClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/Container/ContainerClasses.xml ContainerClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataClasses.xml DataClasses.cpp)

GENERATE_INTROSPECTION_CODE(Function/FunctionClasses.xml FunctionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/MiscFunctions/MiscFunctionsClasses.xml MiscFunctionsClasses.cpp)
GENERATE_INTROSPECTION_CODE(Function/ScalarFunction/ScalarFunctionClasses.xml ScalarFunctionClasses.cpp)
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
  ../include/lbcpp/Data/Predicate.h
  Data/Predicate.cpp
  ../include/lbcpp/Data/ProbabilityDistribution.h
  Data/ProbabilityDistribution.cpp
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
  ../include/lbcpp/Object/ObjectGraph.h
  ../include/lbcpp/Object/Table.h
)

SET(LBCPP_OBJECT_SOURCES
  ObjectGraph.cpp
  Table.cpp
)

SET(LBCPP_UTILITIES_HEADERS
  ../include/lbcpp/Utilities/ProgressCallback.h
  ../include/lbcpp/Utilities/RandomGenerator.h
  ../include/lbcpp/Utilities/RandomVariable.h
  ../include/lbcpp/Utilities/IterationFunction.h
  ../include/lbcpp/Utilities/StoppingCriterion.h
)

SET(LBCPP_UTILITIES_SOURCES
  Utilities.cpp
  IterationFunction.cpp
  StoppingCriterion.cpp
)

SET(LBCPP_FEATURE_GENERATOR_HEADERS
  ../include/lbcpp/FeatureGenerator/StringDictionary.h
  ../include/lbcpp/FeatureGenerator/FeatureDictionary.h
  ../include/lbcpp/FeatureGenerator/FeatureGenerator.h
  ../include/lbcpp/FeatureGenerator/FeatureVisitor.h
  ../include/lbcpp/FeatureGenerator/EditableFeatureGenerator.h
  ../include/lbcpp/FeatureGenerator/SparseVector.h
  ../include/lbcpp/FeatureGenerator/DenseVector.h
  ../include/lbcpp/FeatureGenerator/ContinuousFunction.h
  ../include/lbcpp/FeatureGenerator/Optimizer.h
)

SET(LBCPP_FEATURE_GENERATOR_IMPL ../include/lbcpp/FeatureGenerator/impl)

SET(LBCPP_FEATURE_GENERATOR_IMPL_HEADERS
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitorStatic.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureGeneratorDefaultImplementations.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureGenerator.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/DoubleVector.hpp
)

SET(LBCPP_FEATURE_GENERATOR_IMPL_FEATURE_VISITORS
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/FeatureVisitorDynamicToStatic.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/CreateVectorVisitor.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/UnaryOperationVisitor.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/AssignmentVisitor.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/DotProductVisitor.hpp
  ${LBCPP_FEATURE_GENERATOR_IMPL}/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(LBCPP_FEATURE_GENERATOR_SOURCES
  StringDictionary.cpp
  FeatureDictionary.cpp
  FeatureGenerator.cpp
  FeatureVisitor.cpp
  SparseVector.cpp
  DenseVector.cpp
  DoubleVectorHelpers.hpp
  ContinuousFunction.cpp
  Optimizer.cpp
)

SET(LBCPP_CORE_FEATURE_GENERATOR_SOURCES
  FeatureGenerator/EmptyFeatureGenerator.h
  FeatureGenerator/UnitFeatureGenerator.h
  FeatureGenerator/SubFeatureGenerator.h
  FeatureGenerator/WeightedFeatureGenerator.h
  FeatureGenerator/LinearCombinationFeatureGenerator.h
  FeatureGenerator/ExplicitLinearCombinationFeatureGenerator.h
)

SET(LBCPP_CORE_CONTINUOUS_FUNCTION_SOURCES
  ContinuousFunction/BinarySumScalarVectorFunction.h
  ContinuousFunction/MultiplyByScalarVectorFunction.h
  ContinuousFunction/SumOfSquaresScalarVectorFunction.h
)

SET(LBCPP_CORE_OPTIMIZER_SOURCES
  Optimizer/RPropOptimizer.h
  Optimizer/GradientDescentOptimizer.h
  Optimizer/BackTrackingLineSearch.h
  Optimizer/QuasiNewtonMemory.h
  Optimizer/LBFGSOptimizer.h
)
### old <-------------------


##### FUNCTION ####

SET(LBCPP_FUNCTION_SOURCES
  ../include/lbcpp/Function/Function.h
  ../include/lbcpp/Function/ScalarFunction.h
  Function/ScalarFunction.cpp
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
  ../include/lbcpp/ObjectPredeclarations.h
  ../include/lbcpp/Traits.h
  ../include/lbcpp/ContainerTraits.h
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
  ${LBCPP_OBJECT_SOURCES}
  ${LBCPP_CORE_OBJECT_STREAM_SOURCES}

  ${LBCPP_UTILITIES_SOURCES}

  ${LBCPP_FEATURE_GENERATOR_HEADERS}
  ${LBCPP_FEATURE_GENERATOR_IMPL_HEADERS}
  ${LBCPP_FEATURE_GENERATOR_IMPL_FEATURE_VISITORS}
  ${LBCPP_FEATURE_GENERATOR_SOURCES}
  ${LBCPP_CORE_FEATURE_GENERATOR_SOURCES}
  ${LBCPP_CORE_CONTINUOUS_FUNCTION_SOURCES}
  ${LBCPP_CORE_OPTIMIZER_SOURCES}
  ## <--

  # Function
  ${LBCPP_FUNCTION_SOURCES}
    ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES}
    ${LBCPP_FUNCTION_SCALAR_SOURCES}
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

  classes.cpp
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
SOURCE_GROUP(FeatureGenerator FILES ${LBCPP_FEATURE_GENERATOR_HEADERS} ${LBCPP_FEATURE_GENERATOR_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator impl FILES ${LBCPP_FEATURE_GENERATOR_IMPL_HEADERS})
SOURCE_SUB_GROUP2(FeatureGenerator impl FeatureVisitor FILES ${LBCPP_FEATURE_GENERATOR_IMPL_FEATURE_VISITORS})
SOURCE_SUB_GROUP(FeatureGenerator FeatureGenerator FILES ${LBCPP_CORE_FEATURE_GENERATOR_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator ContinuousFunction FILES ${LBCPP_CORE_CONTINUOUS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator Optimizer FILES ${LBCPP_CORE_OPTIMIZER_SOURCES})
## <---

## Function

SOURCE_GROUP(Function FILES ${LBCPP_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(Function MiscFunctions FILES ${LBCPP_FUNCTION_MISC_FUNCTIONS_SOURCES})
SOURCE_SUB_GROUP(Function ScalarFunction FILES ${LBCPP_FUNCTION_SCALAR_SOURCES})
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
