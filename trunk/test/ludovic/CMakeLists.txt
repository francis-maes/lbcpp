INCLUDE_DIRECTORIES(
  ${Nieme_SOURCE_DIR}/src/cralgorithm/include
)

ADD_CRALGORITHM(GeneratedCode/Test.h Test.cr++)

SET(LUDOCR_SOURCES
  GeneratedCode/Test.h
  Main.cpp
)

ADD_EXECUTABLE(LudoCR ${LUDOCR_SOURCES})
TARGET_LINK_LIBRARIES(LudoCR cralgo-cpp)
