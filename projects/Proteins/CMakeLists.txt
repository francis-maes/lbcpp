INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM_SOURCES(InferenceData/Sequence.lcpp)

SET(LBCPP_INFERENCE_DATA_SOURCES
  InferenceData/AccumulatedScoresMatrix.h
  InferenceData/Sequence.h
  InferenceData/LabelSequence.h
  InferenceData/ScoreVectorSequence.h
)

SET(LBCPP_INFERENCE_STEP_SOURCES
  InferenceStep/InferenceStep.h
  InferenceStep/InferenceStep.cpp
  InferenceStep/SequentialInferenceStep.h
  InferenceStep/ParallelInferenceStep.h
  InferenceStep/ParallelSequenceLabelingInferenceStep.h
  InferenceStep/ClassificationInferenceStep.h
  InferenceStep/DecoratorInferenceStep.h
)

SET(LBCPP_INFERENCE_CONTEXT_SOURCES
  InferenceContext/InferenceStack.h
  InferenceContext/InferenceContext.h
  InferenceContext/InferenceContext.cpp
  InferenceContext/InferenceCallback.h
  InferenceContext/ExamplesCreatorCallback.h
)

SET(LBCPP_INFERENCE_LEARNER_SOURCES
  InferenceLearner/InferenceLearnerCallback.h
  InferenceLearner/InferenceLearner.h
  InferenceLearner/InferenceLearner.cpp
  InferenceLearner/GlobalSimulationInferenceLearner.h
  InferenceLearner/SingleStepDeterministicSimulationInferenceLearner.h
)

SET(LBCPP_PROTEIN_INFERENCE_SOURCES
  ProteinInference/Protein.h
  ProteinInference/AminoAcidDictionary.h
  ProteinInference/AminoAcidDictionary.cpp
  ProteinInference/SecondaryStructureDictionary.h
  ProteinInference/SecondaryStructureDictionary.cpp
  ProteinInference/ProteinInference.h
  ProteinInference/ProteinEvaluationCallback.h
)

SET(PROTEINS_CPP_SOURCES
  ${LBCPP_INFERENCE_DATA_SOURCES}
  ${LBCPP_INFERENCE_STEP_SOURCES}
  ${LBCPP_INFERENCE_CONTEXT_SOURCES}
  ${LBCPP_INFERENCE_LEARNER_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  classes.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_GROUP(InferenceData FILES ${LBCPP_INFERENCE_DATA_SOURCES} InferenceData/Sequence.lcpp)
SOURCE_GROUP(InferenceStep FILES ${LBCPP_INFERENCE_STEP_SOURCES})
SOURCE_GROUP(InferenceContext FILES ${LBCPP_INFERENCE_CONTEXT_SOURCES})
SOURCE_GROUP(InferenceLearner FILES ${LBCPP_INFERENCE_LEARNER_SOURCES})
SOURCE_GROUP(ProteinInference FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

ADD_EXECUTABLE(ProteinsCB513Converter Programs/CB513Converter.cpp)
TARGET_LINK_LIBRARIES(ProteinsCB513Converter proteins)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SandBox Programs/SandBox.lcpp)
TARGET_LINK_LIBRARIES(SandBox proteins)

ADD_CREXECUTABLE(TrainStepByStepSS Programs/TrainStepByStepSS.lcpp)
TARGET_LINK_LIBRARIES(TrainStepByStepSS proteins)
