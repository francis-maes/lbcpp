INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM_SOURCES(InferenceData/Sequence.lcpp)

SET(LBCPP_INFERENCE_DATA_SOURCES
  InferenceData/AccumulatedScoresMatrix.h
  InferenceData/Sequence.h
  InferenceData/LabelSequence.h
  InferenceData/LabelSequence.cpp
  InferenceData/ScoreVectorSequence.h
  InferenceData/ScoreVectorSequence.cpp
  InferenceData/CartesianCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.cpp
  InferenceData/ScoreSymmetricMatrix.h
)

SET(LBCPP_INFERENCE_STEP_SOURCES
  InferenceStep/InferenceStep.h
  InferenceStep/InferenceStep.cpp
  InferenceStep/ClassificationInferenceStep.h
  InferenceStep/RegressionInferenceStep.h
  InferenceStep/SequentialInferenceStep.h
  InferenceStep/SequentialInferenceStep.cpp
  InferenceStep/ParallelInferenceStep.h
  InferenceStep/DecoratorInferenceStep.h
  InferenceStep/DecoratorInferenceStep.cpp
  InferenceStep/ParallelSequenceLabelingInferenceStep.h
  InferenceStep/ParallelSharedMultiRegressionInferenceStep.h
  InferenceStep/ParallelSequenceMultiRegressionInferenceStep.h
)

SET(LBCPP_INFERENCE_CONTEXT_SOURCES
  InferenceContext/InferenceVisitor.h
  InferenceContext/InferenceVisitor.cpp
  InferenceContext/InferenceStack.h
  InferenceContext/InferenceContext.h
  InferenceContext/InferenceContext.cpp
  InferenceContext/InferenceCallback.h
  InferenceContext/CancelAfterStepCallback.h
  InferenceContext/CacheInferenceCallback.h
  InferenceContext/ExamplesCreatorCallback.h
)

SET(LBCPP_INFERENCE_LEARNER_SOURCES
  InferenceLearner/InferenceLearnerCallback.h
  InferenceLearner/InferenceLearner.h
  InferenceLearner/InferenceLearner.cpp
  InferenceLearner/GlobalSimulationInferenceLearner.h
  InferenceLearner/SingleStepDeterministicSimulationInferenceLearner.h
)

SET(LBCPP_PROTEIN_GEOMETRY_SOURCES
  Geometry/DihedralAngle.h
  Geometry/Vector3.h
  Geometry/Matrix4.h
)

SET(LBCPP_PROTEIN_INFERENCE_SOURCES
  ProteinInference/AminoAcidDictionary.h
  ProteinInference/AminoAcidDictionary.cpp
  ProteinInference/SecondaryStructureDictionary.h
  ProteinInference/SecondaryStructureDictionary.cpp
  ProteinInference/ProteinResidue.h
  ProteinInference/ProteinResidue.cpp
  ProteinInference/ProteinTertiaryStructure.h
  ProteinInference/ProteinTertiaryStructure.cpp
  ProteinInference/Protein.h
  ProteinInference/Protein.cpp
  ProteinInference/ProteinInference.h
  ProteinInference/ProteinEvaluationCallback.h
)

SET(LBCPP_PROTEIN_FORMATS_SOURCES
  ProteinInference/Formats/FASTAFileParser.h
  ProteinInference/Formats/FASTAFileGenerator.h
  ProteinInference/Formats/PDBFileParser.h
  ProteinInference/Formats/PDBFileParser.cpp
  ProteinInference/Formats/PDBFileGenerator.h
  ProteinInference/Formats/PDBFileGenerator.cpp
  ProteinInference/Formats/CASPFileGenerator.h
  ProteinInference/Formats/CASPFileGenerator.cpp
  ProteinInference/Formats/OrderDisorderRegionCASPFileGenerator.h
  ProteinInference/Formats/ResidueResidueDistanceCASPFileGenerator.h
  ProteinInference/Formats/TertiaryStructureCASPFileGenerator.h
)

SET(PROTEINS_CPP_SOURCES
  ${LBCPP_INFERENCE_DATA_SOURCES}
  ${LBCPP_INFERENCE_STEP_SOURCES}
  ${LBCPP_INFERENCE_CONTEXT_SOURCES}
  ${LBCPP_INFERENCE_LEARNER_SOURCES}
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  ${LBCPP_PROTEIN_FORMATS_SOURCES}
  classes.cpp
  InferencePredeclarations.h
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
SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})
SOURCE_GROUP(Protein FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})
SOURCE_SUB_GROUP(Protein Formats FILES ${LBCPP_PROTEIN_FORMATS_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

MACRO(ADD_SINGLE_CPP_EXECUTABLE NAME)
  ADD_EXECUTABLE(${NAME} Programs/${NAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} proteins)
ENDMACRO(ADD_SINGLE_CPP_EXECUTABLE)

ADD_SINGLE_CPP_EXECUTABLE(CASPPredictor)
ADD_SINGLE_CPP_EXECUTABLE(ConvertPDBToProtein)
ADD_SINGLE_CPP_EXECUTABLE(ConvertPDBToCASPTS)
ADD_SINGLE_CPP_EXECUTABLE(ComputePSSMs)
ADD_SINGLE_CPP_EXECUTABLE(CompileProteins)
ADD_SINGLE_CPP_EXECUTABLE(TertiaryStructureRepresentationTests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SandBox Programs/SandBox.lcpp)
TARGET_LINK_LIBRARIES(SandBox proteins)

ADD_CREXECUTABLE(TrainStepByStepSS Programs/TrainStepByStepSS.lcpp)
TARGET_LINK_LIBRARIES(TrainStepByStepSS proteins)
