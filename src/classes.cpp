/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareClassClasses();
extern void declareObjectClasses();
extern void declareFunctionClasses();
extern void declareContainerClasses();
extern void declareStreamClasses();
extern void declareConsumerClasses();
extern void declarePredicateClasses();
extern void declareProbabilityDistributionClasses();
extern void declarePerceptionClasses();

// Object
void declareObjectRelatedClasses()
{
  declareObjectClasses();
  declareFunctionClasses();
  declareContainerClasses();
  declareConsumerClasses();
  declareStreamClasses();
  declarePredicateClasses();
  declareProbabilityDistributionClasses();
  declarePerceptionClasses();
}

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

  declareRandomVariableStatistics();
  declareFeatureGenerators();
  declareObjectRelatedClasses();
  
  declareContinuousFunctions();
  declareIterationFunctions();
  declareStoppingCriterions();
  declareOptimizers();
  
  LBCPP_DECLARE_DICTIONARY(BinaryClassificationDictionary);
  
  declareInferenceOnlineLearnerClasses();
  declareInferenceClasses();
  declareInferenceCallbackClasses();
  
}
