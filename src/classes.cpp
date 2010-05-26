/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareGradientBasedLearners();
extern void declareGradientBasedLearningMachines();

// Object
void declareObjectRelatedClasses()
{
  LBCPP_DECLARE_CLASS(StringToObjectMap);
}

// Utilities
extern void declareIterationFunctions();
extern void declareStoppingCriterions();

void declareRandomVariableStatistics()
{
  LBCPP_DECLARE_CLASS(ScalarVariableMean);
  LBCPP_DECLARE_CLASS(ScalarVariableMeanAndVariance);
  LBCPP_DECLARE_CLASS(ScalarVariableStatistics);
}

// FeatureGenerator
extern void declareContinuousFunctions();
extern void declareOptimizers();

void declareFeatureGenerators()
{
  LBCPP_DECLARE_CLASS(StringDictionary);
  LBCPP_DECLARE_CLASS(FeatureDictionary);

  LBCPP_DECLARE_CLASS(Label);
  LBCPP_DECLARE_CLASS(SparseVector);
  LBCPP_DECLARE_CLASS(DenseVector);
}

// CRAlgorithm
extern void declareChooseFunctions();
extern void declarePolicies();
extern void declareCRAlgorithmLearners();

// Inference
extern void declareInferenceClasses();
extern void declareInferenceCallbackClasses();
extern void declareInferenceOnlineLearnerClasses();

void declareLBCppCoreClasses()
{
  declareRandomVariableStatistics();
  declareFeatureGenerators();
  declareObjectRelatedClasses();
  
  declareContinuousFunctions();
  declareIterationFunctions();
  declareStoppingCriterions();
  declareOptimizers();
  
  LBCPP_DECLARE_DICTIONARY(BinaryClassificationDictionary);
  declareGradientBasedLearners();
  declareGradientBasedLearningMachines();
  
  declareChooseFunctions();
  declarePolicies();
  declareCRAlgorithmLearners();

  declareInferenceClasses();
  declareInferenceCallbackClasses();
  declareInferenceOnlineLearnerClasses();
}
