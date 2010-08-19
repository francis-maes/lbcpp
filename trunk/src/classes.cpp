/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

// new
extern void declareDataClasses();

// old
extern void declareClassClasses();
extern void declareContainerClasses();
extern void declarePredicateClasses();
extern void declareProbabilityDistributionClasses();

// Utilities
extern void declareIterationFunctions();
extern void declareStoppingCriterions();

void declareRandomVariableStatistics()
{
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMean);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMeanAndVariance);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableStatistics);
}

// FeatureGenerator
extern void declareContinuousFunctions();
extern void declareOptimizers();

void declareFeatureGenerators()
{
  LBCPP_DECLARE_CLASS_LEGACY(StringDictionary);
  LBCPP_DECLARE_CLASS_LEGACY(FeatureDictionary);

  LBCPP_DECLARE_ABSTRACT_CLASS(FeatureGenerator, Object);

  LBCPP_DECLARE_CLASS_LEGACY(Label);
  LBCPP_DECLARE_CLASS_LEGACY(Scalar);
  LBCPP_DECLARE_CLASS_LEGACY(SparseVector);
  LBCPP_DECLARE_CLASS_LEGACY(DenseVector);
}

// Inference
extern void declareInferenceClasses();
extern void declareInferenceCallbackClasses();
extern void declareInferenceOnlineLearnerClasses();

void declareLBCppCoreClasses()
{
  declareClassClasses();
  declareContainerClasses();
  
  declareDataClasses(); // generated

  declarePredicateClasses();
  declareProbabilityDistributionClasses();

  declareRandomVariableStatistics();
  declareFeatureGenerators();
  
  declareContinuousFunctions();
  declareIterationFunctions();
  declareStoppingCriterions();
  declareOptimizers();
  
  LBCPP_DECLARE_DICTIONARY(BinaryClassificationDictionary);
  
  declareInferenceOnlineLearnerClasses();
  declareInferenceClasses();
  declareInferenceCallbackClasses();
  
}
