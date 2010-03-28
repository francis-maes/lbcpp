INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET(PROTEINS_DATA_GENERIC
  Data/Generic/Sequence.lh
  Data/Generic/LabelSequence.lh
  Data/Generic/ScoreSequence.lh
  Data/Generic/ScoreVectorSequence.lh
)

SET(PROTEINS_DATA_BIO
  Data/Bio/AminoAcid.lh
  Data/Bio/AminoAcid.lcpp
  Data/Bio/AminoAcidSequence.lh
  Data/Bio/PositionSpecificScoringMatrix.lh
  Data/Bio/SecondaryStructureSequence.lh
  Data/Bio/SolventAccesibilitySequence.lh
  Data/Bio/Protein.lh
)

SET(PROTEINS_LBCPP_SOURCES
  ${PROTEINS_DATA_GENERIC}
  ${PROTEINS_DATA_BIO}
)

ADD_CRALGORITHM_SOURCES(${PROTEINS_LBCPP_SOURCES})

SET(PROTEINS_MAIN_SOURCES
  classes.cpp
)

SET(PROTEINS_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_MAIN_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_SUB_GROUP(Data Generic FILES ${PROTEINS_DATA_GENERIC})
SOURCE_SUB_GROUP(Data Bio FILES ${PROTEINS_DATA_BIO})
SOURCE_GROUP(Main FILES ${PROTEINS_MAIN_SOURCES})

ADD_LIBRARY(proteins ${PROTEINS_SOURCES})
TARGET_LINK_LIBRARIES(proteins lbcpp-core)

ADD_EXECUTABLE(ProteinsCB513Converter Programs/CB513Converter.cpp)
TARGET_LINK_LIBRARIES(ProteinsCB513Converter proteins)

ADD_EXECUTABLE(ProteinsTestLearning Programs/TestLearning.cpp)
TARGET_LINK_LIBRARIES(ProteinsTestLearning proteins)
