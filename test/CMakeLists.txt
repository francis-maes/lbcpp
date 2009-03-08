INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

ADD_CRTEST(EmptyCRAlgorithm.cr++)	
ADD_CRTEST(SenseOneFeature.cr++)
ADD_CRTEST(UserTypeTraits.cr++)
ADD_CRTEST(SimplestSequenceLabeling.cr++)

ADD_SUBDIRECTORY(ludovic)

ADD_EXECUTABLE(test-cralgo-cpp TestCpp.cpp)
TARGET_LINK_LIBRARIES(test-cralgo-cpp cralgo-cpp)
