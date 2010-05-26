INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

SET(LBCPP_CORE_IMPL ../include/lbcpp/impl)

##### OBJECT ####

SET(LBCPP_OBJECT_HEADERS
  ../include/lbcpp/Object/ReferenceCountedObject.h
  ../include/lbcpp/Object/ErrorHandler.h  
  ../include/lbcpp/Object/Object.h
  ../include/lbcpp/Object/ObjectStream.h
  ../include/lbcpp/Object/ObjectContainer.h
  ../include/lbcpp/Object/ObjectPair.h
  ../include/lbcpp/Object/ObjectConsumer.h
  ../include/lbcpp/Object/ObjectGraph.h
  ../include/lbcpp/Object/Table.h
  ../include/lbcpp/Object/StringToObjectMap.h
)

SET(LBCPP_OBJECT_SOURCES
  Object.cpp
  ObjectStream.cpp
  ObjectContainer.cpp
  ObjectConsumer.cpp
  ObjectGraph.cpp
  Table.cpp
)

SET(LBCPP_CORE_OBJECT_STREAM_SOURCES
  ObjectStream/ApplyFunctionObjectStream.h
  ObjectStream/ClassificationExamplesParser.h
  ObjectStream/ClassificationExamplesSyntheticGenerator.h
  ObjectStream/RegressionExamplesParser.h
  ObjectStream/DirectoriesObjectPairStream.h
)


##### UTILITIES ####

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

##### FEATURE GENERATOR ####

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
  ContinuousFunction/MiscScalarFunctions.h
  ContinuousFunction/HingeLossFunction.h
  ContinuousFunction/LogBinomialLossFunction.h
  ContinuousFunction/BinarySumScalarVectorFunction.h
  ContinuousFunction/ScalarArchitectureExampleLossFunction.h
  ContinuousFunction/ScalarArchitectureEmpiricalRiskFunction.h
)

SET(LBCPP_CORE_OPTIMIZER_SOURCES
  Optimizer/RPropOptimizer.h
  Optimizer/GradientDescentOptimizer.h
  Optimizer/BackTrackingLineSearch.h
  Optimizer/QuasiNewtonMemory.h
  Optimizer/LBFGSOptimizer.h
)

##### CR-ALGORITHM ####

SET(LBCPP_CRALGORITHM_HEADERS
  ../include/lbcpp/CRAlgorithm/ChooseFunction.h
  ../include/lbcpp/CRAlgorithm/Variable.h
  ../include/lbcpp/CRAlgorithm/Choose.h
  ../include/lbcpp/CRAlgorithm/Callback.h
  ../include/lbcpp/CRAlgorithm/Policy.h
  ../include/lbcpp/CRAlgorithm/CRAlgorithmScope.h
  ../include/lbcpp/CRAlgorithm/CRAlgorithm.h
  ../include/lbcpp/CRAlgorithm/CRAlgorithmLearner.h
)

SET(LBCPP_CRALGORITHM_SOURCES
  ChooseFunction.cpp
  Choose.cpp
  Policy.cpp
  CRAlgorithmLearner.cpp
)

SET(LBCPP_CORE_CHOOSE_FUNCTION_SOURCES
  ChooseFunction/ChooseValueFunctions.h
  ChooseFunction/RegressorValueFunctions.h
  ChooseFunction/ClassifierValueFunctions.h
  ChooseFunction/RankerValueFunctions.h
  ChooseFunction/SimulationStateValueFunction.h
  ChooseFunction/StateValueBasedActionValueFunction.h
)

SET(LBCPP_CORE_POLICY_SOURCES
  Policy/RandomPolicy.h
  Policy/GreedyPolicy.h
  Policy/StochasticPolicy.h
  Policy/EpsilonGreedyPolicy.h
  Policy/GibbsGreedyPolicy.h
  Policy/MixturePolicy.h
  Policy/ComputeStatisticsPolicy.h
  Policy/VerbosePolicy.h
  Policy/ClassificationExamplesCreatorPolicy.h
  Policy/RankingExamplesCreatorPolicy.h
  Policy/QLearningPolicy.h
  Policy/MonteCarloControlPolicy.h
  Policy/GPOMDPPolicy.h
)

SET(LBCPP_CORE_CRALGORITHM_LEARNER_SOURCES
  CRAlgorithmLearner/SearnLearner.h
  CRAlgorithmLearner/PolicyBasedCRAlgorithmLearner.h
  CRAlgorithmLearner/SarsaLearner.h
)

##### INFERENCE ####

SET(LBCPP_INFERENCE_HEADERS
  ../include/lbcpp/Inference/predeclarations.h
  ../include/lbcpp/Inference/Inference.h
  ../include/lbcpp/Inference/InferenceStack.h
  ../include/lbcpp/Inference/InferenceVisitor.h
  ../include/lbcpp/Inference/InferenceCallback.h
  ../include/lbcpp/Inference/InferenceContext.h
  ../include/lbcpp/Inference/InferenceBaseClasses.h
  ../include/lbcpp/Inference/InferenceResultCache.h
  ../include/lbcpp/Inference/InferenceLearner.h
  ../include/lbcpp/Inference/Evaluator.h
)

SET(LBCPP_INFERENCE_SOURCES
  Inference/Inference.cpp
  Inference/InferenceVisitor.cpp
  Inference/InferenceCallback.cpp
  Inference/InferenceContext.cpp
  Inference/InferenceLearner.cpp
  Inference/Evaluator.cpp
)

SET(LBCPP_INFERENCE_INFERENCE_SOURCES
  Inference/Inference/LinearInference.h
  Inference/Inference/BinaryClassificationInference.h
  Inference/Inference/OneAgainstAllClassificationInference.h
  Inference/Inference/TransferFunctionDecoratorInference.h
  Inference/Inference/CallbackBasedDecoratorInference.h
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  Inference/InferenceCallback/CancelAfterStepCallback.h
  Inference/InferenceCallback/CacheInferenceCallback.h
  Inference/InferenceCallback/ExamplesCreatorCallback.h
  Inference/InferenceCallback/GradientDescentLearningCallback.h
  Inference/InferenceCallback/GradientDescentLearningCallback.cpp
  Inference/InferenceCallback/StochasticGradientDescentLearningCallback.h
  Inference/InferenceCallback/MiniBatchGradientDescentLearningCallback.h
  Inference/InferenceCallback/BatchGradientDescentLearningCallback.h
  Inference/InferenceCallback/RandomizerLearningInferenceCallback.h
)

SET(LBCPP_INFERENCE_LEARNER_SOURCES
  Inference/InferenceLearner/GlobalSimulationInferenceLearner.h
  Inference/InferenceLearner/SingleStepDeterministicSimulationInferenceLearner.h
)

SET(LBCPP_INFERENCE_EVALUATOR_SOURCES
  Inference/Evaluator/ObjectContainerEvaluator.h
  Inference/Evaluator/BinaryClassificationConfusionEvaluator.h
  Inference/Evaluator/ClassificationAccuracyEvaluator.h
  Inference/Evaluator/RegressionErrorEvaluator.h
)


##### BAZAR ####

SET(LBCPP_CORE_HEADERS
  ../include/lbcpp/common.h
  ../include/lbcpp/ObjectPredeclarations.h
  ../include/lbcpp/Traits.h
  ../include/lbcpp/ContainerTraits.h
  ../include/lbcpp/LearningExample.h
  ../include/lbcpp/LearningMachine.h
  ../include/lbcpp/GradientBasedLearner.h
  ../include/lbcpp/GradientBasedLearningMachine.h
  ../include/lbcpp/lbcpp.h
)

SET(LBCPP_CORE_IMPL_HEADERS
  ${LBCPP_CORE_IMPL}/Macros.hpp
  ${LBCPP_CORE_IMPL}/Object.hpp
  ${LBCPP_CORE_IMPL}/StaticToDynamic.hpp
  ${LBCPP_CORE_IMPL}/DynamicToStatic.hpp
  ${LBCPP_CORE_IMPL}/impl.h
)

SET(LBCPP_CORE_IMPL_FEATURE_VISITOR
  ${LBCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorStatic.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorStaticToDynamic.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/FeatureVisitorDynamicToStatic.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/CreateVectorVisitor.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/UnaryOperationVisitor.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/AssignmentVisitor.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/DotProductVisitor.hpp
  ${LBCPP_CORE_IMPL}/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(LBCPP_CORE_IMPL_CONTINUOUS_FUNCTION
  ${LBCPP_CORE_IMPL}/ContinuousFunction/BinaryScalarOperation.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/FunctionStatic.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/FunctionPairTraits.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/FunctionStaticToDynamic.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/FunctionDynamicToStatic.hpp

  ${LBCPP_CORE_IMPL}/ContinuousFunction/ScalarFunctions.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/ScalarVectorFunctions.hpp

  ${LBCPP_CORE_IMPL}/ContinuousFunction/LossFunctions.hpp

  ${LBCPP_CORE_IMPL}/ContinuousFunction/AbsoluteLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/SquareLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/PerceptronLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/HingeLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/LogBinomialLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/ExponentialLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/MulticlassLogBinomialLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/AllPairsLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/MostViolatedPairLossFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/BestAgainstAllLossFunction.hpp

  ${LBCPP_CORE_IMPL}/ContinuousFunction/ExampleRiskFunction.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/EmpiricalRiskFunction.hpp

  ${LBCPP_CORE_IMPL}/ContinuousFunction/BiasArchitecture.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/LinearArchitecture.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/MultiLinearArchitecture.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/TransferArchitecture.hpp
  ${LBCPP_CORE_IMPL}/ContinuousFunction/ScalarToVectorArchitecture.hpp
)

SET(LBCPP_CORE_IMPL_BRIDGE
  ${LBCPP_CORE_IMPL}/Bridge/StaticInterface.hpp
  ${LBCPP_CORE_IMPL}/Bridge/FeatureGeneratorDefaultImplementations.hpp
  ${LBCPP_CORE_IMPL}/Bridge/FeatureGenerator.hpp
  ${LBCPP_CORE_IMPL}/Bridge/DoubleVector.hpp
  ${LBCPP_CORE_IMPL}/Bridge/Variable.hpp
  ${LBCPP_CORE_IMPL}/Bridge/ChooseFunction.hpp
  ${LBCPP_CORE_IMPL}/Bridge/CompositeChooseFunctions.hpp
  ${LBCPP_CORE_IMPL}/Bridge/Choose.hpp
  ${LBCPP_CORE_IMPL}/Bridge/Callback.hpp
  ${LBCPP_CORE_IMPL}/Bridge/CRAlgorithmScope.hpp
  ${LBCPP_CORE_IMPL}/Bridge/CRAlgorithm.hpp
)

SET(LBCPP_CORE_SOURCES
  classes.cpp
  LearningExample.cpp
  LearningMachine.cpp
  GradientBasedLearner.cpp
  GradientBasedLearningMachine.cpp
)

SET(LBCPP_CORE_GRADIENT_BASED_LEARNER_SOURCES
  GradientBasedLearner/NonLearnerGradientBasedLearner.h
  GradientBasedLearner/StochasticGradientDescentLearner.h
  GradientBasedLearner/MiniBatchGradientDescentLearner.h
  GradientBasedLearner/BatchGradientBasedLearner.h
  GradientBasedLearner/StochasticToBatchGradientLearner.h
)

SET(LBCPP_CORE_GRADIENT_BASED_MACHINE_SOURCES
  GradientBasedLearningMachine/StaticToDynamicGradientBasedMachine.h
  GradientBasedLearningMachine/GradientBasedRegressor.h
  GradientBasedLearningMachine/GradientBasedClassifier.h
  GradientBasedLearningMachine/GradientBasedBinaryClassifier.h
  GradientBasedLearningMachine/GradientBasedRanker.h
)


SET(LBCPP_CORE
  ${LBCPP_CORE_HEADERS}
  ${LBCPP_CORE_IMPL_HEADERS}
  ${LBCPP_CORE_IMPL_FEATURE_VISITOR}
  ${LBCPP_CORE_IMPL_CONTINUOUS_FUNCTION}
  ${LBCPP_CORE_IMPL_BRIDGE}
  ${LBCPP_CORE_SOURCES}
  ${LBCPP_CORE_GRADIENT_BASED_MACHINE_SOURCES}
  ${LBCPP_CORE_GRADIENT_BASED_LEARNER_SOURCES}
 
  ${LBCPP_OBJECT_HEADERS}
  ${LBCPP_OBJECT_SOURCES}
  ${LBCPP_CORE_OBJECT_STREAM_SOURCES}

  ${LBCPP_UTILITIES_SOURCES}

  ${LBCPP_FEATURE_GENERATOR_HEADERS}
  ${LBCPP_FEATURE_GENERATOR_SOURCES}
  ${LBCPP_CORE_FEATURE_GENERATOR_SOURCES}
  ${LBCPP_CORE_CONTINUOUS_FUNCTION_SOURCES}
  ${LBCPP_CORE_OPTIMIZER_SOURCES}

  ${LBCPP_CRALGORITHM_HEADERS}
  ${LBCPP_CRALGORITHM_SOURCES}
  ${LBCPP_CORE_CHOOSE_FUNCTION_SOURCES}
  ${LBCPP_CORE_POLICY_SOURCES}
  ${LBCPP_CORE_CRALGORITHM_LEARNER_SOURCES}
  
  ${LBCPP_INFERENCE_HEADERS}
  ${LBCPP_INFERENCE_SOURCES}
  ${LBCPP_INFERENCE_INFERENCE_SOURCES}
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
  ${LBCPP_INFERENCE_LEARNER_SOURCES}
  ${LBCPP_INFERENCE_EVALUATOR_SOURCES}
)

SOURCE_GROUP(includes FILES ${LBCPP_CORE_HEADERS})

SOURCE_GROUP(impl FILES ${LBCPP_CORE_IMPL_HEADERS})
SOURCE_SUB_GROUP(impl FeatureVisitor FILES ${LBCPP_CORE_IMPL_FEATURE_VISITOR})
SOURCE_SUB_GROUP(impl ContinuousFunction FILES ${LBCPP_CORE_IMPL_CONTINUOUS_FUNCTION})
SOURCE_SUB_GROUP(impl Bridge FILES ${LBCPP_CORE_IMPL_BRIDGE})
SOURCE_SUB_GROUP(impl sources FILES ${LBCPP_CORE_SOURCES})
SOURCE_SUB_GROUP(impl GradientBasedLearningMachine FILES ${LBCPP_CORE_GRADIENT_BASED_MACHINE_SOURCES})
SOURCE_SUB_GROUP(impl GradientBasedLearner FILES ${LBCPP_CORE_GRADIENT_BASED_LEARNER_SOURCES})

SOURCE_GROUP(Object FILES ${LBCPP_OBJECT_HEADERS} ${LBCPP_OBJECT_SOURCES})
SOURCE_SUB_GROUP(Object ObjectStream FILES ${LBCPP_CORE_OBJECT_STREAM_SOURCES})

SOURCE_GROUP(Utilities FILES ${LBCPP_UTILITIES_HEADERS} ${LBCPP_UTILITIES_SOURCES})

SOURCE_GROUP(FeatureGenerator FILES ${LBCPP_FEATURE_GENERATOR_HEADERS} ${LBCPP_FEATURE_GENERATOR_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator FeatureGenerator FILES ${LBCPP_CORE_FEATURE_GENERATOR_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator ContinuousFunction FILES ${LBCPP_CORE_CONTINUOUS_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(FeatureGenerator Optimizer FILES ${LBCPP_CORE_OPTIMIZER_SOURCES})

SOURCE_GROUP(CRAlgorithm FILES ${LBCPP_CRALGORITHM_HEADERS} ${LBCPP_CRALGORITHM_SOURCES})
SOURCE_SUB_GROUP(CRAlgorithm ChooseFunction FILES ${LBCPP_CORE_CHOOSE_FUNCTION_SOURCES})
SOURCE_SUB_GROUP(CRAlgorithm Policy FILES ${LBCPP_CORE_POLICY_SOURCES})
SOURCE_SUB_GROUP(CRAlgorithm CRAlgorithmLearner FILES ${LBCPP_CORE_CRALGORITHM_LEARNER_SOURCES})

SOURCE_GROUP(Inference FILES ${LBCPP_INFERENCE_HEADERS} ${LBCPP_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Inference Inference FILES ${LBCPP_INFERENCE_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Inference Callback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Inference Learner FILES ${LBCPP_INFERENCE_LEARNER_SOURCES})
SOURCE_SUB_GROUP(Inference Evaluator FILES ${LBCPP_INFERENCE_EVALUATOR_SOURCES})

ADD_LIBRARY(lbcpp-core ${LBCPP_CORE})

TARGET_LINK_LIBRARIES(lbcpp-core juce)
