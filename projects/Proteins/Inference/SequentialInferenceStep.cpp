/*-----------------------------------------.---------------------------------.
| Filename: SequentialInferenceStep.cpp    | Base class for sequential       |
| Author  : Francis Maes                   |   inference                     |
| Started : 17/04/2010 11:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "SequentialInferenceStep.h"
#include <lbcpp/Inference/InferenceContext.h>
using namespace lbcpp;

String SequentialInferenceStep::toString() const
{
  String res = getClassName() + T("(");
  size_t n = getNumSubSteps();
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    res += step->toString();
    if (i < n - 1)
      res += T(", ");
  }
  return res + T(")");
}

void SequentialInferenceStep::accept(InferenceVisitorPtr visitor)
  {visitor->visit(SequentialInferenceStepPtr(this));}

ObjectPtr SequentialInferenceStep::run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  size_t n = getNumSubSteps();
  ObjectPtr currentData = input;
  for (size_t i = 0; i < n; ++i)
  {
    InferenceStepPtr step = getSubStep(i);
    ObjectPtr currentSupervision = supervision ? getSubSupervision(supervision, i) : ObjectPtr();
    currentData = context->runInference(step, currentData, currentSupervision, returnCode);
    if (returnCode != finishedReturnCode)
      return ObjectPtr();
    jassert(currentData);
  }
  return currentData;
}
