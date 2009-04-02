INCLUDE_DIRECTORIES(
  ${Lcpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/GeneratedCode
)

SET(CRPP_ROOT_SOURCES
  Utilities.cr++
  Nieme.cr++
  SandBox.cr++
)

SET(CRPP_EXAMPLES_SOURCES
  Examples/Identity.cr++
  Examples/LCeB.cr++
)

SET(CRPP_DATA_SOURCES
  Data/LabelDictionary.cr++
  Data/LabelSequence.cr++
  Data/ContentSequence.cr++
  Data/ContentGraph.cr++
  Data/LabeledContentGraph.cr++
)

SET(CRPP_PREDICTION_SOURCES 
  Prediction/Classification.cr++
  Prediction/SequenceLabeling.cr++
  Prediction/GraphLabeling.cr++
)

SET(CRPP_SEARCH_SOURCES
  Search/TreeSearchNode.cr++
  Search/TreeSearchSpace.cr++
  Search/SortedTreeSearchNodeMap.cr++
  Search/BestFirstSearch.cr++
)

SET(CRPP_FILES
  ${CRPP_ROOT_SOURCES}
  ${CRPP_EXAMPLES_SOURCES}
  ${CRPP_DATA_SOURCES}
  ${CRPP_PREDICTION_SOURCES}
  ${CRPP_SEARCH_SOURCES}
)

#GET_TARGET_PROPERTY(LCPP_TRANSLATOR_COMMAND LcppTranslator LOCATION)
add_cralgorithm_headers(${CRPP_FILES})
SET(CRPP_GENERATED_SOURCES ${CRALGORITHMS_OUTPUT_FILES})

SOURCE_GROUP(Examples FILES ${CRPP_EXAMPLES_SOURCES})
SOURCE_GROUP(Data FILES ${CRPP_DATA_SOURCES})
SOURCE_GROUP(Prediction FILES ${CRPP_PREDICTION_SOURCES})
SOURCE_GROUP(Search FILES ${CRPP_SEARCH_SOURCES})
SOURCE_GROUP(GeneratedCode FILES ${CRPP_GENERATED_SOURCES})

ADD_LIBRARY(lcpp-lib ${CRPP_FILES} ${CRPP_GENERATED_SOURCES} dummy.cpp)
TARGET_LINK_LIBRARIES(lcpp-lib lcpp-core)
