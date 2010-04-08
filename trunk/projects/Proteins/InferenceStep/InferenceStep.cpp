/*-----------------------------------------.---------------------------------.
| Filename: InferenceStep.cpp              | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "InferenceStep.h"
using namespace lbcpp;

void InferenceStep::accept(InferenceVisitorPtr visitor)
  {visitor->visit(InferenceStepPtr(this));}

InferenceStep::ResultCode InferenceStep::runOnSelfSupervisedExamples(InferencePolicyPtr policy, ObjectContainerPtr examples)
  {return runOnSupervisedExamples(policy, examples->apply(new ObjectToObjectPairFunction()));}

InferenceStep::ResultCode InferenceStep::runOnSupervisedExamples(InferencePolicyPtr policy, ObjectContainerPtr examples)
{
  for (size_t i = 0; i < examples->size(); ++i)
  {
    ObjectPairPtr example = examples->get(i).dynamicCast<ObjectPair>();
    jassert(example);
    ObjectPtr correctOutput = example->getSecond();
    ResultCode res = run(policy, example->getFirst(), correctOutput);
    if (res == errorReturnCode)
      return res;
  }
  return finishedReturnCode;
}
