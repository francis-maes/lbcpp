INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(Image/ImageLibrary.xml ImageLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Program/ProgramLibrary.xml ProgramLibrary.cpp)
GENERATE_INTROSPECTION_CODE(MNISTLibrary.xml MNISTLibrary.cpp)

SET(LBCPP_MNIST_IMAGE_SOURCES
  Image/Image.h
  Image/ImageFunction.h
  Image/ImagePerception.h
  Image/ImageLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ImageLibrary.cpp
)

SET(LBCPP_MNIST_PROGRAM_SOURCES
  Program/LearnerProgram.h
  Program/LearnerProgram.cpp
  Program/PredictorProgram.h
  Program/PredictorProgram.cpp
  Program/BinarizerProgram.h
  Program/ProgramLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ProgramLibrary.cpp
)

SET(LBCPP_MNIST_SOURCES
  MatlabFileParser.h
  MNISTLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MNISTLibrary.cpp
)

SOURCE_GROUP(Image FILES ${LBCPP_MNIST_IMAGE_SOURCES})
SOURCE_GROUP(Program FILES ${LBCPP_MNIST_PROGRAM_SOURCES})

ADD_LIBRARY(mnist SHARED 
  ${LBCPP_MNIST_SOURCES}
  ${LBCPP_MNIST_IMAGE_SOURCES}
  ${LBCPP_MNIST_PROGRAM_SOURCES}
)
TARGET_LINK_LIBRARIES(mnist lbcpp-core)

#ADD_EXECUTABLE(mnistLearner MainLearner.cpp)
#TARGET_LINK_LIBRARIES(mnistLearner mnist)

#ADD_EXECUTABLE(mnistPredictor MainPredictor.cpp)
#TARGET_LINK_LIBRARIES(mnistPredictor mnist)

#ADD_EXECUTABLE(mnistConvertDataToBinaryFile MainConvertDataToBinaryFile.cpp)
#TARGET_LINK_LIBRARIES(mnistConvertDataToBinaryFile mnist)