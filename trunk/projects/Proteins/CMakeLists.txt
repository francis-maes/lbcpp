INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

OPTION(LBCPP_PROTEIN_ROSETTA "Enable Rosetta Dependent Code" OFF)


GENERATE_INTROSPECTION_CODE(Geometry/GeometryLibrary.xml GeometryLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/ProteinDataLibrary.xml ProteinDataLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Predictor/ProteinPredictorLibrary.xml ProteinPredictorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Evaluator/ProteinEvaluatorLibrary.xml ProteinEvaluatorLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnit/ProteinWorkUnitLibrary.xml ProteinWorkUnitLibrary.cpp)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/ProteinUserInterfaceLibrary.xml ProteinUserInterfaceLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

GENERATE_INTROSPECTION_CODE(Rosetta/ProteinRosettaLibrary.xml ProteinRosettaLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Rosetta/ProteinMover/ProteinRosettaMoverLibrary.xml ProteinRosettaMoverLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Rosetta/ProteinOptimizer/ProteinRosettaOptimizerLibrary.xml ProteinRosettaOptimizerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Rosetta/Sampler/ProteinRosettaSamplerLibrary.xml ProteinRosettaSamplerLibrary.cpp)

GENERATE_INTROSPECTION_CODE(ProteinLibrary.xml ProteinLibrary.cpp)

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
  Geometry/GeometryLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/GeometryLibrary.cpp
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
  Data/ProteinDataLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinDataLibrary.cpp
)


SET(LBCPP_PROTEIN_DATA_FORMATS_SOURCES
  Data/Formats/FASTAFileParser.h
  Data/Formats/FASTAFileGenerator.h
  Data/Formats/SPXFileParser.h
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

SET(LBCPP_PROTEIN_PREDICTOR_SOURCES
  Predictor/ProteinPredictor.h
  Predictor/ProteinPredictor.cpp
  Predictor/ProteinPerception.h
  Predictor/ProteinPredictorParameters.h
  Predictor/DecoratorProteinPredictorParameters.h
  Predictor/NumericalProteinPredictorParameters.h
  Predictor/NumericalCysteinPredictorParameters.h
  Predictor/Lin09PredictorParameters.h
  Predictor/ConnectivityPatternClassifier.h
  Predictor/LargeProteinPredictorParameters.h
  Predictor/ProteinPredictorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinPredictorLibrary.cpp
)

SET(LBCPP_PROTEIN_EVALUATOR_SOURCES
  Evaluator/TertiaryStructureEvaluator.h
  Evaluator/ContactMapEvaluator.h
  Evaluator/DisulfideBondEvaluator.h
  Evaluator/ProteinEvaluator.h
  Evaluator/QScoreEvaluator.h
  Evaluator/QScoreEvaluator.cpp
  Evaluator/ProteinEvaluatorLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinEvaluatorLibrary.cpp
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
  WorkUnit/ProteinLearner.h
  WorkUnit/ProteinLearner.cpp
  WorkUnit/BricoBox.h
  WorkUnit/BricoBox.cpp
  WorkUnit/CysteinBox.h
  WorkUnit/LargeBox.h
  WorkUnit/ProteinStatisticsWorkUnit.h
  WorkUnit/FabienBox.h
  WorkUnit/ProteinWorkUnitLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinWorkUnitLibrary.cpp
)

IF(LBCPP_USER_INTERFACE)
  SET(LBCPP_PROTEIN_USER_INTERFACE_SOURCES
    UserInterface/MultiSequenceComponent.h
    UserInterface/MultiProtein1DComponent.h
    UserInterface/MultiProtein2DComponent.h
    UserInterface/MultiProteinComponent.h
    UserInterface/ProteinUserInterfaceLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/ProteinUserInterfaceLibrary.cpp
  )
ENDIF(LBCPP_USER_INTERFACE)

SET(LBCPP_PROTEIN_ROSETTA_SOURCES
  Rosetta/RosettaProtein.h
  Rosetta/RosettaTest.h
  Rosetta/RosettaSandBox.h
  Rosetta/RosettaUtils.h
  Rosetta/RosettaUtils.cpp
  Rosetta/ProteinMover.h
  Rosetta/Sampler.h
  Rosetta/ProteinOptimizer.h
  Rosetta/RosettaWorkUnit.h
  Rosetta/ProteinEDAOptimizer.h
  Rosetta/ProteinRosettaLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinRosettaLibrary.cpp
)

SET(LBCPP_PROTEIN_ROSETTA_MOVER_SOURCES
  Rosetta/ProteinMover/PhiPsiMover.h
  Rosetta/ProteinMover/ShearMover.h
  Rosetta/ProteinMover/RigidBodyMover.h
  Rosetta/ProteinMover/ProteinRosettaMoverLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinRosettaMoverLibrary.cpp
)

SET(LBCPP_PROTEIN_ROSETTA_OPTIMIZER_SOURCES
  Rosetta/ProteinOptimizer/GreedyOptimizer.h
  Rosetta/ProteinOptimizer/MonteCarloOptimizer.h
  Rosetta/ProteinOptimizer/SimulatedAnnealingOptimizer.h
  Rosetta/ProteinOptimizer/SequentialOptimizer.h
  Rosetta/ProteinOptimizer/ProteinRosettaOptimizerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinRosettaOptimizerLibrary.cpp
)

  SET(LBCPP_PROTEIN_ROSETTA_SAMPLER_SOURCES
  Rosetta/Sampler/ConditionalProteinMoverSampler.h
  Rosetta/Sampler/GeneralProteinMoverSampler.h
  Rosetta/Sampler/SmoothEnumerationSampler.h
  Rosetta/Sampler/ParzenContinuousSampler.h
  Rosetta/Sampler/SimpleResidueSampler.h
  Rosetta/Sampler/ResiduePairSampler.h
  Rosetta/Sampler/ProteinMoverSampler.h
  Rosetta/Sampler/ProteinRosettaSamplerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinRosettaSamplerLibrary.cpp
)

IF(LBCPP_PROTEIN_ROSETTA)
  ADD_DEFINITIONS(-DLBCPP_PROTEIN_ROSETTA)

  
  IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(ROSETTA_BUILD_TYPE "debug")
  ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(ROSETTA_BUILD_TYPE "release")
  ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

IF(WIN32)
#  ADD_SUBDIRECTORY(Rosetta/rosetta)

  include(FindBoost)
  find_package(
	  Boost
	  1.36.0
	  REQUIRED
  )
  #link_directories(${Boost_LIBRARY_DIRS})
  include_directories(${Boost_INCLUDE_DIRS})
  include_directories(Rosetta/rosetta)

  SET(LBCPP_PROTEIN_ROSETTA_LIBRARIES )
ELSE(WIN32)

  IF(APPLE)
    
    # Mac
    INCLUDE_DIRECTORIES(
      /Users/alex/Documents/Ulg/2M/rosetta3.2_macos/rosetta_source/src
      /Users/alex/Documents/Ulg/2M/rosetta3.2_macos/rosetta_source/src/platform/macos
      /Users/alex/Documents/Ulg/2M/rosetta3.2_macos/rosetta_source/external/boost_1_38_0
    )
    SET(LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE /Users/alex/Documents/Ulg/2M/rosetta3.2_macos/rosetta_source/build/src/${ROSETTA_BUILD_TYPE}/macos/10.6/64/x86/gcc)
    SET(LBCPP_PROTEIN_ROSETTA_LIBRARIES ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE}/libcore.dylib ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE}/libutility.dylib ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE}/libnumeric.dylib ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE}/libObjexxFCL.dylib ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_APPLE}/libprotocols.dylib)
    
  ELSE(APPLE)

    # Linux
    INCLUDE_DIRECTORIES(
      ${LBCpp_SOURCE_DIR}/rosetta3.2-src/rosetta_source/src
      ${LBCpp_SOURCE_DIR}/rosetta3.2-src/rosetta_source/src/platform/linux
    )
    SET(LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_LINUX ${LBCpp_SOURCE_DIR}/rosetta3.2-src/rosetta_source/build/src/release/linux/2.6/64/x86/gcc)
    SET(LBCPP_PROTEIN_ROSETTA_LIBRARIES ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_LINUX}/libcore.so ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_LINUX}/libnumeric.so ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_LINUX}/libObjexxFCL.so ${LBCPP_PROTEIN_ROSETTA_LIBRARIES_DIR_LINUX}/libprotocols.so)

  ENDIF(APPLE)
ENDIF(WIN32)

ENDIF(LBCPP_PROTEIN_ROSETTA)


SET(PROTEINS_CPP_SOURCES
  ${LBCPP_PROTEIN_GEOMETRY_SOURCES}
  ${LBCPP_PROTEIN_DATA_SOURCES}
  ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES}
  ${LBCPP_PROTEIN_PERCEPTION_SOURCES}
  ${LBCPP_PROTEIN_PREDICTOR_SOURCES}
  ${LBCPP_PROTEIN_INFERENCE_SOURCES}
  ${LBCPP_PROTEIN_EVALUATOR_SOURCES}
  ${LBCPP_PROTEIN_WORK_UNIT_SOURCES}
  ${LBCPP_PROTEIN_USER_INTERFACE_SOURCES}
  ${LBCPP_PROTEIN_ROSETTA_SOURCES}
  ${LBCPP_PROTEIN_ROSETTA_MOVER_SOURCES}
  ${LBCPP_PROTEIN_ROSETTA_OPTIMIZER_SOURCES}
  ${LBCPP_PROTEIN_ROSETTA_SAMPLER_SOURCES}
  
  ProteinLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProteinLibrary.cpp
)

SET(PROTEINS_SOURCES
  precompiled.h
  precompiled.cpp
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_CPP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})

SOURCE_GROUP(Geometry FILES ${LBCPP_PROTEIN_GEOMETRY_SOURCES})

SOURCE_GROUP(Data FILES ${LBCPP_PROTEIN_DATA_SOURCES})
SOURCE_SUB_GROUP(Data Formats FILES ${LBCPP_PROTEIN_DATA_FORMATS_SOURCES})
SOURCE_GROUP(Predictor FILES ${LBCPP_PROTEIN_PREDICTOR_SOURCES})
SOURCE_GROUP(Evaluator FILES ${LBCPP_PROTEIN_EVALUATOR_SOURCES})
SOURCE_GROUP(WorkUnit FILES ${LBCPP_PROTEIN_WORK_UNIT_SOURCES})
SOURCE_GROUP(UserInterface FILES ${LBCPP_PROTEIN_USER_INTERFACE_SOURCES})
SOURCE_GROUP(Rosetta FILES ${LBCPP_PROTEIN_ROSETTA_SOURCES})
SOURCE_SUB_GROUP(Rosetta ProteinMover FILES ${LBCPP_PROTEIN_ROSETTA_MOVER_SOURCES})
SOURCE_SUB_GROUP(Rosetta ProteinOptimizer FILES ${LBCPP_PROTEIN_ROSETTA_OPTIMIZER_SOURCES})
SOURCE_SUB_GROUP(Rosetta Sampler FILES ${LBCPP_PROTEIN_ROSETTA_SAMPLER_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled PROTEINS_SOURCES)

ADD_LIBRARY(proteins${NAME_FILE_SUFFIX} SHARED ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX} ${LBCPP_PROTEIN_ROSETTA_LIBRARIES})
