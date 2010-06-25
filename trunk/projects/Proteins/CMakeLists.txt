INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM_SOURCES(
  InferenceData/CommonFeatureGenerators.lcpp
  InferenceData/Sequence.lcpp
  Protein/ProteinBackboneBondSequence.lcpp
  Protein/Inference/ProteinGlobalFeatures.lcpp
  Protein/Inference/ProteinResidueFeatures.lcpp
  Protein/Inference/ProteinResiduePairFeatures.lcpp
)

SET(LBCPP_INFERENCE_DATA_SOURCES
  InferenceData/CommonFeatureGenerators.h
  InferenceData/AccumulatedScoresMatrix.h
  InferenceData/Sequence.h
  InferenceData/ScalarSequence.h
  InferenceData/ScalarSequence.cpp
  InferenceData/LabelSequence.h
  InferenceData/LabelSequence.cpp
  InferenceData/ScoreVectorSequence.h
  InferenceData/ScoreVectorSequence.cpp
  InferenceData/CartesianCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.h
  InferenceData/BondCoordinatesSequence.cpp
  InferenceData/ScoreSymmetricMatrix.h
  InferenceData/ScoreSymmetricMatrix.cpp
)

SET(LBCPP_PROTEIN_GEOMETRY_SOURCES
  Geometry/DihedralAngle.h
  Geometry/Vector3.h
  Geometry/Matrix3.h
  Geometry/Matrix3.cpp  
  Geometry/Matrix4.h
  Geometry/Matrix4.cpp
  Geometry/kdtree_lib.h
  Geometry/kdtree_lib.c
)

SET(LBCPP_PROTEIN_SOURCES
# new:
  Protein/AminoAcid.h
  Protein/AminoAcid.cpp
  Protein/SecondaryStructure.h
  Protein/SecondaryStructure.cpp
  
# old:
  Protein/AminoAcidDictionary.h
  Protein/AminoAcidDictionary.cpp
  Protein/SecondaryStructureDictionary.h
  Protein/SecondaryStructureDictionary.cpp
  
# -
  Protein/ProteinBackboneBondSequence.h
  Protein/ProteinResidueAtoms.h
  Protein/ProteinResidueAtoms.cpp
  Protein/ProteinTertiaryStructure.h
  Protein/ProteinTertiaryStructure.cpp
  Protein/Protein.h
  Protein/Protein.cpp
)

SET(LBCPP_PROTEIN_INFERENCE_STEP_SOURCES
  Protein/Inference/ProteinGlobalFeatures.h
  Protein/Inference/ProteinResidueFeatures.h
  Protein/Inference/ProteinResiduePairFeatures.h
  Protein/Inference/ProteinTargetInferenceHelper.h
  Protein/Inference/Protein1DTargetInference.h
  Protein/Inference/ProteinCAlphaBondSequenceInference.h
  Protein/Inference/ProteinBackboneBondSequenceInference.h
  Protein/Inference/ProteinTertiaryStructureInference.h
  Protein/Inference/Protein2DTargetInference.h
  Protein/Inference/ProteinContactMapInference.h
  Protein/Inference/ProteinContactMapInference.cpp
  Protein/Inference/ProteinInference.h
)

SET(LBCPP_PROTEIN_EVALUATION_SOURCES
  Protein/Evaluation/ScoreVectorSequenceErrorEvaluator.h
  Protein/Evaluation/ProteinEvaluationCallback.h
  Protein/Evaluation/ProteinStatisticsCalculator.h
)

SET(LBCPP_PROTEIN_FORMATS_SOURCES
  Protein/Formats/FASTAFileParser.h
  Protein/Formats/FASTAFileGenerator.h
  Protein/Formats/PDBFileParser.h
  Protein/Formats/PDBFileParser.cpp
  Protein/Formats/PDBFileGenerator.h
  Protein/Formats/PDBFileGenerator.cpp
  Protein/Formats/CASPFileGenerator.h
  Protein/Formats/CASPFileGenerator.cpp
  Protein/Formats/OrderDisorderRegionCASPFileGenerator.h
  Protein/Formats/ResidueResidueDistanceCASPFileGenerator.h
  Protein/Formats/TertiaryStructureCASPFileGenerator.h
  Protein/Formats/PSSMFileParser.h
  Protein/Formats/DSSPFileParser.h
)

SET(PROTEINS_CPP_SOURCES
  ${LBCPP_INFERENCE_DATA_SOURCES}
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES}
  ${LBCPP_PROTEIN_EVALUATION_SOURCES}
  ${LBCPP_PROTEIN_FORMATS_SOURCES}
  classes.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})

SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})
SOURCE_GROUP(GenericData FILES ${LBCPP_INFERENCE_DATA_SOURCES} InferenceData/CommonFeatureGenerators.lcpp InferenceData/Sequence.lcpp)
SOURCE_GROUP(ProteinData FILES ${LBCPP_PROTEIN_SOURCES} Protein/ProteinBackboneBondSequence.lcpp)
SOURCE_GROUP(ProteinInference FILES ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES} Protein/Inference/ProteinGlobalFeatures.lcpp Protein/Inference/ProteinResidueFeatures.lcpp Protein/Inference/ProteinResiduePairFeatures.lcpp)
SOURCE_GROUP(ProteinEvaluation FILES ${LBCPP_PROTEIN_EVALUATION_SOURCES})
SOURCE_GROUP(ProteinFormats FILES ${LBCPP_PROTEIN_FORMATS_SOURCES})

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
ADD_SINGLE_CPP_EXECUTABLE(ProteinInferenceEvaluator)
ADD_SINGLE_CPP_EXECUTABLE(TertiaryStructureRepresentationTests)
ADD_SINGLE_CPP_EXECUTABLE(ConvertCASPToValidPDB)
ADD_SINGLE_CPP_EXECUTABLE(ExtractSequenceFromProtein)
ADD_SINGLE_CPP_EXECUTABLE(AttributeValueSandBox)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SandBox Programs/SandBox.lcpp)
TARGET_LINK_LIBRARIES(SandBox proteins)

ADD_CRALGORITHM_SOURCES(Programs/SunBox.lcpp)
SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
ADD_EXECUTABLE(SunBox ${CRALGORITHMS_OUTPUT_FILES} Programs/ArgumentSet.h)
TARGET_LINK_LIBRARIES(SunBox proteins lbcpp-core)
