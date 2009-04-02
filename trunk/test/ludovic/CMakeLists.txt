INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

ADD_CRALGORITHM(GeneratedCode/Test.h Test.cr++)

SET(LUDOCR_SOURCES
  GeneratedCode/Test.h
  Main.cpp
)

ADD_EXECUTABLE(LudoCR ${LUDOCR_SOURCES})
TARGET_LINK_LIBRARIES(LudoCR lcpp-core)
