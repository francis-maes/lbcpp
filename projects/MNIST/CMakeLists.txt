INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(MNISTClasses.xml MNISTClasses.cpp)

SET(LBCPP_MNIST_SOURCES
  LearnerProgram.h
  LearnerProgram.cpp
  MNISTImage.h
  MNISTPerception.h
  MatlabFileParser.h
  PredictorProgram.h
  PredictorProgram.cpp
  MNISTClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/MNISTClasses.cpp
)

ADD_LIBRARY(mnist ${LBCPP_MNIST_SOURCES})
TARGET_LINK_LIBRARIES(mnist lbcpp-core)

SET(LBCPP_MNIST_LEARNER_SOURCES
  MainLearner.cpp
)

SET(LBCPP_MNIST_PREDICTOR_SOURCES
  MainPredictor.cpp
)

ADD_EXECUTABLE(mnistLearner MainLearner.cpp)
TARGET_LINK_LIBRARIES(mnistLearner mnist)

ADD_EXECUTABLE(mnistPredictor MainPredictor.cpp)
TARGET_LINK_LIBRARIES(mnistPredictor mnist)
