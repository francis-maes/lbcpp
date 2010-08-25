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
extern void declareFunctionClasses();
extern void declareInferenceClasses();


// old
extern void declarePredicateClasses();
extern void declareProbabilityDistributionClasses();
extern void declareIterationFunctions();
extern void declareStoppingCriterions();

void declareRandomVariableStatistics()
{
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMean);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMeanAndVariance);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableStatistics);
}

// Inference
extern void declareInferenceCallbackClasses();

void declareLBCppCoreClasses()
{
  declareDataClasses(); // generated
  
  // old -->
  declarePredicateClasses();
  declareProbabilityDistributionClasses();
  declareRandomVariableStatistics();
  declareIterationFunctions();
  declareStoppingCriterions();
  // <--  
  
  declareFunctionClasses(); // generated
  declareInferenceClasses(); // generated

  Type::finishDeclarations();
}
