INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

IF(CRALGO_TESTS)

ADD_CRTEST(EmptyCRAlgorithm.cr++)	
ADD_CRTEST(SenseOneFeature.cr++)
ADD_CRTEST(UserTypeTraits.cr++)
ADD_CRTEST(SimplestSequenceLabeling.cr++)

ENDIF(CRALGO_TESTS)

ADD_SUBDIRECTORY(ludovic)
ADD_SUBDIRECTORY(SequentialClassification)

ADD_EXECUTABLE(test-cralgo TestCpp.cpp)
TARGET_LINK_LIBRARIES(test-cralgo cralgo)
