INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

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

SET(LBCPP_PROTEIN_PERCEPTION_SOURCES
  Perception/ProteinPerception.h
  Perception/ProteinPerception.cpp
  Perception/ResiduePerception.h
  Perception/ResiduePerception.cpp
  Perception/ResiduePairPerception.h
  Perception/ResiduePairPerception.cpp
  Perception/CommonFeatureGenerators.h
  Perception/PerceptionToFeatures.h
)

SET(LBCPP_PROTEIN_INFERENCE_SOURCES
  Inference/ContactMapInference.h
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

ADD_CRALGORITHM_SOURCES(
  Perception/PerceptionToFeatures.lcpp
  Perception/CommonFeatureGenerators.lcpp
)


SET(PROTEINS_CPP_SOURCES
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_DATA_SOURCES}
  ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES}
  ${LBCPP_PROTEIN_PERCEPTION_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  ${LBCPP_PROTEIN_EVALUATOR_SOURCES}
  
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
SOURCE_GROUP(Perception FILES ${LBCPP_PROTEIN_PERCEPTION_SOURCES} Perception/PerceptionToFeatures.lcpp)
SOURCE_GROUP(Inference FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})
SOURCE_GROUP(Evaluator FILES ${LBCPP_PROTEIN_EVALUATOR_SOURCES})

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
ADD_SINGLE_CPP_EXECUTABLE(ConvertCASPToValidPDB)
ADD_SINGLE_CPP_EXECUTABLE(ExtractSequenceFromProtein)
ADD_SINGLE_CPP_EXECUTABLE(MoonBox)
ADD_SINGLE_CPP_EXECUTABLE(SandBox)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})


