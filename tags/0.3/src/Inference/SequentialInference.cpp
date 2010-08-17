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
namespace lbcpp
{

ClassPtr sequentialInferenceClass()
  {static TypeCache cache(T("SequentialInference")); return cache();}

ClassPtr staticSequentialInferenceClass()
  {static TypeCache cache(T("StaticSequentialInference")); return cache();}


class VectorSequentialInferenceClass : public DynamicClass
{
public:
  VectorSequentialInferenceClass()
    : DynamicClass(T("VectorSequentialInference"), staticSequentialInferenceClass())
  {
    addVariable(vectorClass(inferenceClass()), T("subInferences"));
  }

  virtual VariableValue create() const
    {return new VectorSequentialInference();}

  LBCPP_DECLARE_VARIABLE_BEGIN(VectorSequentialInference)
    LBCPP_DECLARE_VARIABLE(subInferences);
  LBCPP_DECLARE_VARIABLE_END()
};

}; /* namespace lbcpp */

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

void declareSequentialInferenceClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(SequentialInference, Inference);
    LBCPP_DECLARE_ABSTRACT_CLASS(StaticSequentialInference, SequentialInference);
      Type::declare(new VectorSequentialInferenceClass());
}
