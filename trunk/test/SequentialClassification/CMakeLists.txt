INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

ADD_CRALGORITHM(GeneratedCode/SequentialClassification.h SequentialClassification.lh)

SET(SEQUENTIAL_CLASSIFICATION_SOURCES
  GeneratedCode/SequentialClassification.h
  SyntheticDataGenerator.h
  SequentialClassification.cpp
)

ADD_EXECUTABLE(SequentialClassification ${SEQUENTIAL_CLASSIFICATION_SOURCES})
TARGET_LINK_LIBRARIES(SequentialClassification cralgo)
