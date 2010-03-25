INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

#SET(PROTEINS_LHEADERS
#  ContentGraph.lh
#  LabelSequence.lh
#)
#ADD_CRALGORITHM_SOURCES(${PROTEINS_LHEADERS})

SET(PROTEINS_GROUP_SOURCES
#  GraphLabelingAlgorithm/GraphLabelingAlgorithm.h
#  GraphLabelingAlgorithm/ClassifierBasedGraphLabelingAlgorithm.h
#  GraphLabelingAlgorithm/IterativeClassificationAlgorithm.h
#  GraphLabelingAlgorithm/GibbsSamplingAlgorithm.h
#  GraphLabelingAlgorithm/StackedGraphLabelingAlgorithm.h
#  GraphLabelingAlgorithm/CRAlgorithmGraphLabelingAlgorithm.h
#  LabeledContentGraph.cpp
  Main.cpp
)

SET(PROTEINS_SOURCES
#  ${CRALGORITHMS_OUTPUT_FILES}
  ${PROTEINS_GROUP_SOURCES}
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_GROUP(ProteinsGroup FILES ${PROTEINS_GROUP_SOURCES})

ADD_EXECUTABLE(Proteins ${PROTEINS_SOURCES})

TARGET_LINK_LIBRARIES(Proteins lbcpp-core)
