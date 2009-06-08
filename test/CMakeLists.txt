INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

IF(LBCPP_TESTS)

ADD_CRTEST(EmptyCRAlgorithm.cr++)	
ADD_CRTEST(SenseOneFeature.cr++)
ADD_CRTEST(UserTypeTraits.cr++)
ADD_CRTEST(SimplestSequenceLabeling.cr++)

ENDIF(LBCPP_TESTS)

#ADD_SUBDIRECTORY(ludovic)
ADD_SUBDIRECTORY(SequentialClassification)
ADD_SUBDIRECTORY(GraphLabeling)
ADD_SUBDIRECTORY(SemanticRoleLabeling)
ADD_SUBDIRECTORY(Search)

ADD_EXECUTABLE(test-lbcpp TestCpp.cpp)
TARGET_LINK_LIBRARIES(test-lbcpp lbcpp-core)
