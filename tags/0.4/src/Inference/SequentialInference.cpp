/*-----------------------------------------.---------------------------------.
| Filename: SequentialInference.cpp        | Sequential Inference            |
| Author  : Francis Maes                   |   base classes                  |
| Started : 26/11/2010 14:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** SequentialInference
*/
Variable SequentialInference::computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  SequentialInferenceStatePtr state = prepareInference(context, input, supervision);
  if (!state)
    return Variable();

  while (!state->isFinal())
  {
    Variable subOutput = state->getSubInference()->run(context, state->getSubInput(), state->getSubSupervision());
    state->setSubOutput(subOutput);
    if (!updateInference(context, state))
      state->setFinalState();
  }
  return finalizeInference(context, state);
}

/*
** StaticSequentialInference
*/
StaticSequentialInference::StaticSequentialInference(const String& name)
  : SequentialInference(name)
{
  setBatchLearner(staticSequentialInferenceLearner());
}

/*
** VectorSequentialInference
*/
VectorSequentialInference::VectorSequentialInference(const String& name)
  : StaticSequentialInference(name) {}

SequentialInferenceStatePtr VectorSequentialInference::prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
  if (subInferences.size())
    prepareSubInference(context, state, 0);
  return state;
}

bool VectorSequentialInference::updateInference(ExecutionContext& context, SequentialInferenceStatePtr state) const
{
  int index = state->getStepNumber(); 
  jassert(index >= 0);
  finalizeSubInference(context, state, (size_t)index);
  jassert(state->getSubInference() == getSubInference(index));
  ++index;
  if (index < (int)subInferences.size())
  {
    prepareSubInference(context, state, (size_t)index);
    return true;
  }
  else
    return false;
}

void VectorSequentialInference::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  ReferenceCountedObjectPtr<VectorSequentialInference> target = t.staticCast<VectorSequentialInference>();
  StaticSequentialInference::clone(context, target);
  jassert(target->subInferences.size() == subInferences.size());
  for (size_t i = 0; i < subInferences.size(); ++i)
    target->subInferences[i] = subInferences[i]->cloneAndCast<Inference>(context);
}
