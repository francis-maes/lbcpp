INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

IF(LCPP_TESTS)

ADD_CRTEST(EmptyCRAlgorithm.cr++)	
ADD_CRTEST(SenseOneFeature.cr++)
ADD_CRTEST(UserTypeTraits.cr++)
ADD_CRTEST(SimplestSequenceLabeling.cr++)

ENDIF(LCPP_TESTS)

ADD_SUBDIRECTORY(ludovic)
ADD_SUBDIRECTORY(SequentialClassification)
ADD_SUBDIRECTORY(GraphLabeling)

ADD_EXECUTABLE(test-lcpp TestCpp.cpp)
TARGET_LINK_LIBRARIES(test-lcpp lcpp)
