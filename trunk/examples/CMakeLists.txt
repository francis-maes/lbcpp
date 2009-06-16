INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

MACRO(ADD_CPP_EXAMPLE CATEGORY FILENAME)
  SET(NAME ${CATEGORY}-${FILENAME})
  ADD_EXECUTABLE(${NAME} ${CATEGORY}/${FILENAME}.cpp)
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CPP_EXAMPLE)

MACRO(ADD_LBCPP_EXAMPLE CATEGORY FILENAME)
  SET(NAME ${CATEGORY}-${FILENAME})
  ADD_CREXECUTABLE(${NAME} ${CATEGORY}/${FILENAME}.lcpp)
ENDMACRO(ADD_LBCPP_EXAMPLE)

ADD_CPP_EXAMPLE(LearningMachine TrainMaxentClassifierBatch)
ADD_CPP_EXAMPLE(LearningMachine TrainMaxentClassifierStochastic)
ADD_CPP_EXAMPLE(LearningMachine TrainMaxentClassifierOnline)
ADD_CPP_EXAMPLE(LearningMachine LoadMaxentClassifier)
ADD_CPP_EXAMPLE(LearningMachine SyntheticLinearRegression)
ADD_CPP_EXAMPLE(LearningMachine TuneMaxentRegularizerWithCrossValidation)
ADD_CPP_EXAMPLE(LearningMachine ClassificationToBatchRanking)

ADD_LBCPP_EXAMPLE(CRAlgorithm FirstOrderSequenceLabeling)

