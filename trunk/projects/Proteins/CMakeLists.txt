INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM_SOURCES(InferenceData/Sequence.lcpp)

SET(LBCPP_INFERENCE_DATA_SOURCES
  InferenceData/Sequence.h
  InferenceData/LabelSequence.h
  InferenceData/ScoreVectorSequence.h
)

SET(LBCPP_INFERENCE_STEP_SOURCES
  InferenceStep/InferenceStep.h
  InferenceStep/InferenceStep.cpp
  InferenceStep/SequentialInferenceStep.h
  InferenceStep/ParallelInferenceStep.h
  InferenceStep/ClassificationInferenceStep.h
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  InferenceCallback/InferenceCallback.h
  InferenceCallback/InferenceContext.h
  InferenceCallback/InferenceContext.cpp
  InferenceCallback/ExamplesCreatorCallback.h
  InferenceCallback/GlobalSimulationLearningCallback.h
  InferenceCallback/SingleStepSimulationLearningCallback.h
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
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
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
SOURCE_GROUP(InferenceCallback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})
SOURCE_GROUP(ProteinInference FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

ADD_EXECUTABLE(ProteinsCB513Converter Programs/CB513Converter.cpp)
TARGET_LINK_LIBRARIES(ProteinsCB513Converter proteins)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SecondaryStructureCrossValidation Programs/SecondaryStructureCrossValidation.lcpp)
TARGET_LINK_LIBRARIES(SecondaryStructureCrossValidation proteins)

ADD_CREXECUTABLE(MultiPassSecondaryStructure Programs/MultiPassSecondaryStructure.lcpp)
TARGET_LINK_LIBRARIES(MultiPassSecondaryStructure proteins)

ADD_CREXECUTABLE(FromScratch Programs/FromScratch.lcpp)
TARGET_LINK_LIBRARIES(FromScratch proteins)
