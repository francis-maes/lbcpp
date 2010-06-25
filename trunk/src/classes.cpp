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

  LBCPP_DECLARE_CLASS_LEGACY(StringToObjectMap);
  declareObjectContainerClasses();
  declareObjectStreamClasses();
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
