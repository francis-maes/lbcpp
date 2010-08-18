/*-----------------------------------------.---------------------------------.
| Filename: ParallelInference.cpp          | Parallel Inference              |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 22:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/DecoratorInference.h>
using namespace lbcpp;

/*
** StaticParallelInference
*/
StaticParallelInference::StaticParallelInference(const String& name)
  : ParallelInference(name)
{
  setBatchLearner(staticParallelInferenceLearner());
}


/*
** SharedParallelInference
*/
SharedParallelInference::SharedParallelInference(const String& name, InferencePtr subInference)
  : StaticParallelInference(name), subInference(subInference)
{
  setBatchLearner(sharedParallelInferenceLearner());
}

Variable SharedParallelInference::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  subInference->beginRunSession();
  Variable res = ParallelInference::run(context, input, supervision, returnCode);
  subInference->endRunSession();
  return res;
}

String SharedParallelInference::toString() const
{
  jassert(subInference);
  return getClassName() + T("(") + subInference->toString() + T(")");
}
