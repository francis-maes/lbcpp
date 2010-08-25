/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareDataClasses();
extern void declareFunctionClasses();
extern void declareInferenceClasses();

// old
extern void declareProbabilityDistributionClasses();
void declareRandomVariableStatistics()
{
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMean);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableMeanAndVariance);
  LBCPP_DECLARE_CLASS_LEGACY(ScalarVariableStatistics);
}
// --

void declareLBCppCoreClasses()
{
  declareDataClasses(); // generated
  
  // old -->
  declareProbabilityDistributionClasses();
  declareRandomVariableStatistics();
  // <--  
  
  declareFunctionClasses(); // generated
  declareInferenceClasses(); // generated

  Type::finishDeclarations();
}
