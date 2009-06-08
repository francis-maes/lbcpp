INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

MACRO(ADD_CPP_EXAMPLE NAME FILE)
  ADD_EXECUTABLE(${NAME} ${FILE})
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CPP_EXAMPLE)

ADD_CPP_EXAMPLE(LearningMachine-TrainMaxentClassifierBatch LearningMachine/TrainMaxentClassifierBatch.cpp)
ADD_CPP_EXAMPLE(LearningMachine-TrainMaxentClassifierStochastic LearningMachine/TrainMaxentClassifierStochastic.cpp)
ADD_CPP_EXAMPLE(LearningMachine-TrainMaxentClassifierOnline LearningMachine/TrainMaxentClassifierOnline.cpp)
