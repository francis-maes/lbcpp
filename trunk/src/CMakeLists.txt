INCLUDE_DIRECTORIES(${CRAlgo_SOURCE_DIR}/include)

SET(CRALGO_LIBRARY_HEADERS
  ../include/cralgo/cralgo.h
  ../include/cralgo/Traits.h
  ../include/cralgo/ContainerTraits.h
  ../include/cralgo/ReferenceCountedObject.h
  ../include/cralgo/Object.h
  ../include/cralgo/ErrorHandler.h
  ../include/cralgo/StringDictionary.h
  ../include/cralgo/FeatureDictionary.h
  ../include/cralgo/FeatureGenerator.h
  ../include/cralgo/DoubleVector.h
  ../include/cralgo/SparseVector.h
  ../include/cralgo/DenseVector.h
  ../include/cralgo/LazyVector.h
  ../include/cralgo/LearningExample.h
  ../include/cralgo/ContinuousFunction.h
  ../include/cralgo/StateFunction.h
  ../include/cralgo/Choose.h
  ../include/cralgo/Callback.h
  ../include/cralgo/Policy.h
  ../include/cralgo/CRAlgorithmScope.h
  ../include/cralgo/CRAlgorithm.h
  ../include/cralgo/impl/impl.h
)

SET(CRALGO_LIBRARY_IMPL_FEATURE_VISITOR
  ../include/cralgo/impl/FeatureVisitor/StaticFeatureVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/CreateSparseVectorVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/UnaryOperationVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/AssignmentVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/DotProductVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/DynamicToStaticFeatureVisitor.hpp
  ../include/cralgo/impl/FeatureVisitor/StringDescriptionVisitor.hpp
)

SET(CRALGO_LIBRARY_IMPL_FUNCTION
  ../include/cralgo/impl/Function/ContinuousFunction.hpp
  ../include/cralgo/impl/Function/FunctionPairTraits.hpp
  ../include/cralgo/impl/Function/FunctionInstantiate.hpp
  ../include/cralgo/impl/Function/ScalarContinuousFunction.hpp
  ../include/cralgo/impl/Function/ScalarDerivableFunction.hpp
  ../include/cralgo/impl/Function/ScalarVectorContinuousFunction.hpp
  ../include/cralgo/impl/Function/ScalarVectorDerivableFunction.hpp
  ../include/cralgo/impl/Function/LossFunction.hpp
  ../include/cralgo/impl/Function/ScalarDerivableArchitecture.hpp
  ../include/cralgo/impl/Function/VectorDerivableArchitecture.hpp
)

SET(CRALGO_LIBRARY_IMPL_BRIDGE
  ../include/cralgo/impl/Bridge/StaticInterface.hpp
  ../include/cralgo/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp
  ../include/cralgo/impl/Bridge/FeatureGenerator.hpp
  ../include/cralgo/impl/Bridge/DoubleVector.hpp
  ../include/cralgo/impl/Bridge/StateFunction.hpp
  ../include/cralgo/impl/Bridge/Choose.hpp
  ../include/cralgo/impl/Bridge/Callback.hpp
  ../include/cralgo/impl/Bridge/CRAlgorithmScope.hpp
  ../include/cralgo/impl/Bridge/CRAlgorithm.hpp
)

SET(CRALGO_LIBRARY_SOURCES
  classes.cpp
  Object.cpp
  ErrorHandler.cpp
  StringDictionary.cpp
  SparseVector.cpp
  DenseVector.cpp
  DoubleVectorHelpers.hpp
  LearningExample.cpp
)

SET(CRALGO_LIBRARY
  ${CRALGO_LIBRARY_HEADERS}
  ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR}
  ${CRALGO_LIBRARY_IMPL_FUNCTION}
  ${CRALGO_LIBRARY_IMPL_BRIDGE}
  ${CRALGO_LIBRARY_SOURCES}
)

SOURCE_GROUP(includes FILES ${CRALGO_LIBRARY_HEADERS})
SOURCE_GROUP(impl/FeatureVisitor FILES ${CRALGO_LIBRARY_IMPL_FEATURE_VISITOR})
SOURCE_GROUP(impl/Function FILES ${CRALGO_LIBRARY_IMPL_FUNCTION})
SOURCE_GROUP(impl/Bridge FILES ${CRALGO_LIBRARY_IMPL_BRIDGE})
SOURCE_GROUP(sources FILES ${CRALGO_LIBRARY_SOURCES})

ADD_LIBRARY(cralgo ${CRALGO_LIBRARY})
