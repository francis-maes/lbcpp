INCLUDE_DIRECTORIES(
  ${CRAlgo_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM(GeneratedCode/SequentialClassification.h SequentialClassification.lh)

SET(GRAPH_LABELING_LHEADERS
  ContentGraph.lh
  LabelSequence.lh
  LabeledContentGraph.lh
  GraphLabelingCRAlgorithms.lh
)

ADD_CRALGORITHM_SOURCES(${GRAPH_LABELING_LHEADERS})

SET(GRAPH_LABELING_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  LabeledContentGraph.cpp
  Main.cpp
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})

ADD_EXECUTABLE(GraphLabeling ${GRAPH_LABELING_SOURCES})

TARGET_LINK_LIBRARIES(GraphLabeling cralgo)
