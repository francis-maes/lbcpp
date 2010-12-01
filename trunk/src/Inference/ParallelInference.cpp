/*-----------------------------------------.---------------------------------.
| Filename: ParallelInference.cpp          | Parallel Inference              |
| Author  : Francis Maes                   |   base classes                  |
| Started : 26/11/2010 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
#include <lbcpp/Execution/WorkUnit.h>
using namespace lbcpp;

/*
** ParallelInference
*/
Variable ParallelInference::computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  ParallelInferenceStatePtr state = prepareInference(context, input, supervision);
  if (!state)
    return Variable();

  size_t n = state->getNumSubInferences();
  
  if (context.isMultiThread() && useMultiThreading())
  {
    CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(getName(), n));
    for (size_t i = 0; i < n; ++i)
    {
      String description = state->getSubInference(i)->getDescription(context, state->getSubInput(i), state->getSubSupervision(i));
      workUnits->setWorkUnit(i, new InferenceWorkUnit(description, state->getSubInference(i), state->getSubInput(i), state->getSubSupervision(i), &state->getSubOutput(i)));
    }
    context.run(workUnits);
  }
  else
  {
    for (size_t i = 0; i < n; ++i)
    {
      Variable subOutput;
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        if (!subInference->run(context, state->getSubInput(i), state->getSubSupervision(i), &subOutput))
        {
          context.errorCallback("ParallelInference::computeInference", "Could not finish sub inference");
          return Variable(); 
        }
      }
      state->setSubOutput(i, subOutput);
    }
  }
  return finalizeInference(context, state);
}

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

Variable SharedParallelInference::computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  subInference->beginRunSession();
  Variable res = ParallelInference::computeInference(context, input, supervision);
  subInference->endRunSession();
  return res;
}

String SharedParallelInference::toString() const
{
  jassert(subInference);
  return getClassName() + T("(") + subInference->toString() + T(")");
}

/*
** VectorParallelInference
*/
void VectorParallelInference::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  ReferenceCountedObjectPtr<VectorParallelInference> target = t.staticCast<VectorParallelInference>();
  StaticParallelInference::clone(context, target);
  jassert(target->subInferences.size() == subInferences.size());
  for (size_t i = 0; i < subInferences.size(); ++i)
    target->subInferences[i] = subInferences[i]->cloneAndCast<Inference>(context);
}
