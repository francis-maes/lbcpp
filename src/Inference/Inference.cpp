/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
using namespace lbcpp;

void Inference::clone(ObjectPtr target) const
{
  NameableObject::clone(target);
  if (onlineLearner)
    InferencePtr(target)->onlineLearner = onlineLearner->cloneAndCast<InferenceOnlineLearner>();
}

extern void declareInferenceClasses();
extern void declareReductionInferenceClasses();
extern void declareDecisionTreeInferenceClasses();
extern void declareNumericalInferenceClasses();
extern void declareMetaInferenceClasses();

void declareInferenceLibrary()
{
  declareInferenceClasses();
  declareReductionInferenceClasses();
  declareDecisionTreeInferenceClasses();
  declareNumericalInferenceClasses();
  declareMetaInferenceClasses();
}
