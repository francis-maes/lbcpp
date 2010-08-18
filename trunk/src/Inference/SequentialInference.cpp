/*-----------------------------------------.---------------------------------.
| Filename: SequentialInference.cpp        | Sequential Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 22:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/SequentialInference.h>
using namespace lbcpp;

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
  : StaticSequentialInference(name), subInferences(new Vector(inferenceClass())) {}

SequentialInferenceStatePtr VectorSequentialInference::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
  if (subInferences->size())
    prepareSubInference(context, state, 0, returnCode);
  return state;
}

bool VectorSequentialInference::updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
{
  int index = state->getStepNumber(); 
  jassert(index >= 0);
  finalizeSubInference(context, state, (size_t)index, returnCode);
  jassert(state->getSubInference() == getSubInference(index));
  ++index;
  if (index < (int)subInferences->size())
  {
    prepareSubInference(context, state, (size_t)index, returnCode);
    return true;
  }
  else
    return false;
}
