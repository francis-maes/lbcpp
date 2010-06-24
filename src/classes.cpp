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

extern void declareObjectContainerClasses();
extern void declareObjectStreamClasses();

// Object
void declareObjectRelatedClasses()
{
  declareClassClasses();

  LBCPP_DECLARE_CLASS(StringToObjectMap);
  declareObjectContainerClasses();
  declareObjectStreamClasses();
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
  LBCPP_DECLARE_CLASS(Scalar);
  LBCPP_DECLARE_CLASS(SparseVector);
  LBCPP_DECLARE_CLASS(DenseVector);
}

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
  
  declareInferenceClasses();
  declareInferenceCallbackClasses();
  declareInferenceOnlineLearnerClasses();
}
