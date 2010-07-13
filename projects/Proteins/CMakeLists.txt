INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)


### new
SET(LBCPP_PROTEIN_GEOMETRY_SOURCES
  Geometry/DihedralAngle.h
  Geometry/Vector3.h
  Geometry/Vector3.cpp
  Geometry/Matrix3.h
  Geometry/Matrix3.cpp  
  Geometry/Matrix4.h
  Geometry/Matrix4.cpp
  Geometry/kdtree_lib.h
  Geometry/kdtree_lib.c
)

SET(LBCPP_PROTEIN_DATA_SOURCES
  Data/AminoAcid.h
  Data/AminoAcid.cpp
  Data/SecondaryStructure.h
  Data/SecondaryStructure.cpp
  Data/CartesianPositionVector.h
  Data/CartesianPositionVector.cpp
  Data/Atom.h
  Data/Atom.cpp
  Data/Residue.h
  Data/Residue.cpp
  Data/TertiaryStructure.h
  Data/TertiaryStructure.cpp
  Data/Protein.h
  Data/Protein.cpp
)


SET(LBCPP_PROTEIN_DATA_FORMATS_SOURCES
  Data/Formats/FASTAFileParser.h
  Data/Formats/FASTAFileGenerator.h
  Data/Formats/PDBFileParser.h
  Data/Formats/PDBFileParser.cpp
  Data/Formats/PDBFileGenerator.h
  Data/Formats/PDBFileGenerator.cpp
  Data/Formats/CASPFileGenerator.h
  Data/Formats/CASPFileGenerator.cpp
  Data/Formats/OrderDisorderRegionCASPFileGenerator.h
  Data/Formats/ResidueResidueDistanceCASPFileGenerator.h
  Data/Formats/TertiaryStructureCASPFileGenerator.h
  Data/Formats/PSSMFileParser.h
  Data/Formats/DSSPFileParser.h
)

SET(LBCPP_PROTEIN_INFERENCE_SOURCES
  Inference/ProteinInference.h
  Inference/ProteinInference.cpp
  Inference/ProteinInferenceFactory.h
  Inference/ProteinInferenceFactory.cpp
)

SET(LBCPP_PROTEIN_EVALUATOR_SOURCES
  Evaluator/TertiaryStructureEvaluator.h
  Evaluator/ContactMapEvaluator.h
  Evaluator/ProteinEvaluator.h
)

###
### old
###

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


SET(LBCPP_PROTEIN_OLD_DATA_SOURCES
  Protein/AminoAcidDictionary.h
  Protein/AminoAcidDictionary.cpp
  Protein/SecondaryStructureDictionary.h
  Protein/SecondaryStructureDictionary.cpp
  Protein/ProteinBackboneBondSequence.h
  Protein/ProteinResidueAtoms.h
  Protein/ProteinResidueAtoms.cpp
  Protein/ProteinTertiaryStructure.h
  Protein/ProteinTertiaryStructure.cpp
  Protein/ProteinObject.h
  Protein/ProteinObject.cpp
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

SET(PROTEINS_CPP_SOURCES
  # new 
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_DATA_SOURCES}
  ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  ${LBCPP_PROTEIN_EVALUATOR_SOURCES}
    
  # old
  ${LBCPP_INFERENCE_DATA_SOURCES}
  ${LBCPP_PROTEIN_OLD_DATA_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES}
  ${LBCPP_PROTEIN_EVALUATION_SOURCES}
  # -
  
  classes.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})

SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})

SOURCE_GROUP(Data FILES ${LBCPP_PROTEIN_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Formats FILES ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES})
SOURCE_GROUP(Inference FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})
SOURCE_GROUP(Evaluator FILES ${LBCPP_PROTEIN_EVALUATOR_SOURCES})

SOURCE_SUB_GROUP(Old Data FILES ${LBCPP_PROTEIN_OLD_DATA_SOURCES} Protein/ProteinBackboneBondSequence.lcpp)
SOURCE_SUB_GROUP(Old GenericData FILES ${LBCPP_INFERENCE_DATA_SOURCES} InferenceData/CommonFeatureGenerators.lcpp InferenceData/Sequence.lcpp)

SOURCE_SUB_GROUP(Old Inference FILES ${LBCPP_PROTEIN_INFERENCE_STEP_SOURCES} Protein/Inference/ProteinGlobalFeatures.lcpp Protein/Inference/ProteinResidueFeatures.lcpp Protein/Inference/ProteinResiduePairFeatures.lcpp)
SOURCE_SUB_GROUP(Old Evaluation FILES ${LBCPP_PROTEIN_EVALUATION_SOURCES})

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
ADD_SINGLE_CPP_EXECUTABLE(ConvertProteinsToNewFormat)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_CREXECUTABLE(SandBox Programs/SandBox.lcpp)
TARGET_LINK_LIBRARIES(SandBox proteins)

ADD_CREXECUTABLE(AttributeValueSandBox Programs/AttributeValueSandBox.lcpp)
TARGET_LINK_LIBRARIES(AttributeValueSandBox proteins)


ADD_CRALGORITHM_SOURCES(Programs/SunBox.lcpp)
SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
ADD_EXECUTABLE(SunBox ${CRALGORITHMS_OUTPUT_FILES} Programs/ArgumentSet.h)
TARGET_LINK_LIBRARIES(SunBox proteins lbcpp-core)
