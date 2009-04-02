INCLUDE_DIRECTORIES(
  ${CRAlgo_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
)

ADD_CRALGORITHM(GeneratedCode/SequentialClassification.h SequentialClassification.lh)

SET(GRAPH_LABELING_LHEADERS
  ContentGraph.lh
  LabelSequence.lh
  LabeledContentGraph.lh
  crIterativeClassification.lh
  crOrderFreeGraphLabeling.lh
)

ADD_CRALGORITHM_SOURCES(${GRAPH_LABELING_LHEADERS})

SET(GRAPH_LABELING_ALGORITHM_SOURCES
  GraphLabelingAlgorithm/GraphLabelingAlgorithm.h
  GraphLabelingAlgorithm/ClassifierBasedGraphLabelingAlgorithm.h
  GraphLabelingAlgorithm/IterativeClassificationAlgorithm.h
  GraphLabelingAlgorithm/GibbsSamplingAlgorithm.h
  GraphLabelingAlgorithm/StackedGraphLabelingAlgorithm.h
  GraphLabelingAlgorithm/CRAlgorithmGraphLabelingAlgorithm.h
)

SET(GRAPH_LABELING_SOURCES
  ${CRALGORITHMS_OUTPUT_FILES}
  ${GRAPH_LABELING_ALGORITHM_SOURCES}
  LabeledContentGraph.cpp
  Main.cpp
)

SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
SOURCE_GROUP(GraphLabelingAlgorithm FILES ${GRAPH_LABELING_ALGORITHM_SOURCES})

ADD_EXECUTABLE(GraphLabeling ${GRAPH_LABELING_SOURCES})

TARGET_LINK_LIBRARIES(GraphLabeling lcpp)
