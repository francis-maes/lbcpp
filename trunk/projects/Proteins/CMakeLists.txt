INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(Geometry/GeometryClasses.xml GeometryClasses.cpp)
GENERATE_INTROSPECTION_CODE(Data/ProteinDataClasses.xml ProteinDataClasses.cpp)
GENERATE_INTROSPECTION_CODE(Perception/ProteinPerceptionClasses.xml ProteinPerceptionClasses.cpp)
GENERATE_INTROSPECTION_CODE(Inference/ProteinInferenceClasses.xml ProteinInferenceClasses.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/ProteinWorkUnitClasses.xml ProteinWorkUnitClasses.cpp)
GENERATE_INTROSPECTION_CODE(UserInterface/ProteinUserInterfaceClasses.xml ProteinUserInterfaceClasses.cpp)
GENERATE_INTROSPECTION_CODE(ProteinClasses.xml ProteinClasses.cpp)

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
  Geometry/CartesianPositionVector.h
  Geometry/CartesianPositionVector.cpp
  Geometry/GeometryClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/GeometryClasses.cpp
)

SET(LBCPP_PROTEIN_DATA_SOURCES
  Data/AminoAcid.h
  Data/AminoAcid.cpp
  Data/SecondaryStructure.h
  Data/Atom.h
  Data/Atom.cpp
  Data/Residue.h
  Data/Residue.cpp
  Data/TertiaryStructure.h
  Data/TertiaryStructure.cpp
  Data/Protein.h
  Data/Protein.cpp
  Data/ProteinFunctions.h
  Data/ProteinDataClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinDataClasses.cpp
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
  Perception/ResidueToResiduePairPerception.h
  Perception/DisulfideBondsResiduePerception.h
  Perception/SeparationDistanceResiduePairPerception.h
  Perception/ProteinPerceptionClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinPerceptionClasses.cpp
)

SET(LBCPP_PROTEIN_INFERENCE_SOURCES
  Inference/ContactMapInference.h
  Inference/DisulfideBondsInference.h
  Inference/ProteinInference.h
  Inference/ProteinInference.cpp
  Inference/ProteinInferenceFactory.h
  Inference/ProteinInferenceFactory.cpp
  Inference/ProteinInferenceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinInferenceClasses.cpp
)

SET(LBCPP_PROTEIN_EVALUATOR_SOURCES
  Evaluator/TertiaryStructureEvaluator.h
  Evaluator/ContactMapEvaluator.h
  Evaluator/ProteinEvaluator.h
)

SET(LBCPP_PROTEIN_WORK_UNIT_SOURCES
  WorkUnit/CaspPDBtoValidPDBWorkUnit.cpp
  WorkUnit/CaspPDBtoValidPDBWorkUnit.h
  WorkUnit/CompileProteinsWorkUnit.cpp
  WorkUnit/CompileProteinsWorkUnit.h
  WorkUnit/ConvertPDBToProteinWorkUnit.cpp
  WorkUnit/ConvertPDBToProteinWorkUnit.h
  WorkUnit/ProteinInferenceEvaluator.cpp
  WorkUnit/ProteinInferenceEvaluatorWorkUnit.cpp
  WorkUnit/ProteinInferenceEvaluatorWorkUnit.h
  WorkUnit/SandBoxWorkUnit.h
  WorkUnit/SandBoxWorkUnit.cpp
  WorkUnit/SnowBox.cpp
  WorkUnit/SnowBox.h
  WorkUnit/ProteinWorkUnitClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinWorkUnitClasses.cpp
)

SET(LBCPP_PROTEIN_USER_INTERFACE_SOURCES
  UserInterface/MultiSequenceComponent.h
  UserInterface/MultiProtein1DComponent.h
  UserInterface/MultiProtein2DComponent.h
  UserInterface/ProteinPerceptionComponent.h
  UserInterface/MultiProteinComponent.h
  UserInterface/ProteinUserInterfaceClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinUserInterfaceClasses.cpp
)

SET(PROTEINS_CPP_SOURCES
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_DATA_SOURCES}
  ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES}
  ${LBCPP_PROTEIN_PERCEPTION_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  ${LBCPP_PROTEIN_EVALUATOR_SOURCES}
  ${LBCPP_PROTEIN_WORK_UNIT_SOURCES}
  ${LBCPP_PROTEIN_USER_INTERFACE_SOURCES}
  
  ProteinClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinClasses.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})

SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})

SOURCE_GROUP(Data FILES ${LBCPP_PROTEIN_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Formats FILES ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES})
SOURCE_GROUP(Perception FILES ${LBCPP_PROTEIN_PERCEPTION_SOURCES})
SOURCE_GROUP(Inference FILES ${LBCPP_PROTEIN_INFERENCE_SOURCES})
SOURCE_GROUP(Evaluator FILES ${LBCPP_PROTEIN_EVALUATOR_SOURCES})
SOURCE_GROUP(WorkUnit FILES ${LBCPP_PROTEIN_WORK_UNIT_SOURCES})
SOURCE_GROUP(UserInterface FILES ${LBCPP_PROTEIN_USER_INTERFACE_SOURCES})

ADD_LIBRARY(proteins SHARED ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

#ADD_EXECUTABLE(BoincEvaluator Programs/BoincEvaluator.cpp)
#TARGET_LINK_LIBRARIES(BoincEvaluator} proteins)