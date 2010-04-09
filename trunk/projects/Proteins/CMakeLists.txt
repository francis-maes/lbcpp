INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET(PROTEINS_DATA_GENERIC
  Data/Generic/VariableSet.lh
  Data/Generic/Sequence.lh
  Data/Generic/LabelSequence.lh
  Data/Generic/ScoreSequence.lh
  Data/Generic/ScoreVectorSequence.lh
)

SET(PROTEINS_DATA_BIO_LBCPP
  Data/Bio/AminoAcid.lh
  Data/Bio/AminoAcidSequence.lh
  Data/Bio/PositionSpecificScoringMatrix.lh
  Data/Bio/SecondaryStructureSequence.lh
  Data/Bio/SolventAccesibilitySequence.lh
  Data/Bio/Protein.lh
)

SET(PROTEINS_DATA_BIO_CPP
  Data/Bio/AminoAcid.cpp
)

SET(PROTEINS_DATA_BIO
  ${PROTEINS_DATA_BIO_LBCPP}
  ${PROTEINS_DATA_BIO_CPP}
)

SET(PROTEINS_LBCPP_SOURCES
  ${PROTEINS_DATA_GENERIC}
  ${PROTEINS_DATA_BIO_LBCPP}
)

ADD_CRALGORITHM_SOURCES(${PROTEINS_LBCPP_SOURCES})

SET(VARIABLE_SET_MODEL_SOURCES
  VariableSetModel/IndependantClassificationVariableSetModel.h
  VariableSetModel/IterativeClassificationVariableSetModel.h
  VariableSetModel/StackedClassificationVariableSetModel.h
  VariableSetModel/SimulatedIterativeClassificationVariableSetModel.h
)

SET(LBCPP_INFERENCE_STEP_SOURCES
  InferenceStep/CommonObjectFunctions.h
  InferenceStep/CommonObjectFunctions.cpp
  InferenceStep/InferenceStep.h
  InferenceStep/InferenceStep.cpp
  InferenceStep/ParallelInferenceStep.h
  InferenceStep/SequenceInferenceStep.h
  InferenceStep/ProteinInferenceSteps.h
)

SET(LBCPP_INFERENCE_CALLBACK_SOURCES
  InferenceCallback/InferencePolicy.h
  InferenceCallback/InferencePolicy.cpp
  InferenceCallback/DecoratorInferencePolicy.h
  InferenceCallback/ExamplesCreatorPolicy.h
  InferenceCallback/GlobalSimulationLearningPolicy.h
  InferenceCallback/ProteinEvaluationPolicy.h
)

SET(PROTEINS_CPP_SOURCES
  ${PROTEINS_DATA_BIO_CPP}
  ${VARIABLE_SET_MODEL_SOURCES}
  ${LBCPP_INFERENCE_STEP_SOURCES}
  ${LBCPP_INFERENCE_CALLBACK_SOURCES}
  VariableSetModel.h
  VariableSetModel.cpp
  classes.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_SUB_GROUP(Data Generic FILES ${PROTEINS_DATA_GENERIC})
SOURCE_SUB_GROUP(Data Bio FILES ${PROTEINS_DATA_BIO})
SOURCE_GROUP(VariableSetModel FILES ${VARIABLE_SET_MODEL_SOURCES})
SOURCE_GROUP(InferenceStep FILES ${LBCPP_INFERENCE_STEP_SOURCES})
SOURCE_GROUP(InferenceCallback FILES ${LBCPP_INFERENCE_CALLBACK_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

ADD_EXECUTABLE(ProteinsCB513Converter Programs/CB513Converter.cpp)
TARGET_LINK_LIBRARIES(ProteinsCB513Converter proteins)

ADD_EXECUTABLE(VariableSetToSequenceData Programs/VariableSetToSequenceData.cpp)
TARGET_LINK_LIBRARIES(VariableSetToSequenceData proteins)

ADD_EXECUTABLE(ProteinsTestLearning Programs/TestLearning.cpp)
TARGET_LINK_LIBRARIES(ProteinsTestLearning proteins)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SecondaryStructureCrossValidation Programs/SecondaryStructureCrossValidation.lcpp)
TARGET_LINK_LIBRARIES(SecondaryStructureCrossValidation proteins)

ADD_CREXECUTABLE(MultiPassSecondaryStructure Programs/MultiPassSecondaryStructure.lcpp)
TARGET_LINK_LIBRARIES(MultiPassSecondaryStructure proteins)

ADD_CREXECUTABLE(FromScratch Programs/FromScratch.lcpp)
TARGET_LINK_LIBRARIES(FromScratch proteins)
