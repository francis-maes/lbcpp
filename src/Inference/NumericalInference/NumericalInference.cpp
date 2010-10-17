/*-----------------------------------------.---------------------------------.
| Filename: NumericalInference.cpp         | Numerical Inference             |
| Author  : Francis Maes                   |   base class                    |
| Started : 30/09/2010 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NumericalInference.h"
#include <lbcpp/Function/ScalarObjectFunction.h>
#include <lbcpp/Perception/PerceptionMaths.h>
using namespace lbcpp;

NumericalInference::NumericalInference(const String& name, PerceptionPtr perception)
  : Inference(name), perception(perception) {}

void NumericalInference::clone(ObjectPtr t) const
{
  const NumericalInferencePtr& target = t.staticCast<NumericalInference>();
  ScopedReadLock _(parametersLock);
  Inference::clone(target);
  if (parameters)
    target->parameters = parameters->deepClone();
}

ObjectPtr NumericalInference::getParametersCopy() const
{
  ScopedReadLock _(parametersLock);
  return parameters ? parameters->deepClone() : ObjectPtr();
}

void NumericalInference::addWeightedToParameters(const ObjectPtr& value, double weight)
{
  {
    ScopedWriteLock _(parametersLock);
    lbcpp::addWeighted(parameters, value, weight);
  }
  validateParametersChange();
}

void NumericalInference::applyRegularizerToParameters(ScalarObjectFunctionPtr regularizer, double weight)
{
  {
    ScopedWriteLock _(parametersLock);
    if (parameters)
      regularizer->compute(parameters, NULL, &parameters, weight);
  }
  validateParametersChange();
}

void NumericalInference::setParameters(ObjectPtr parameters)
{
  {
    ScopedWriteLock _(parametersLock);
    this->parameters = parameters;
  }
  validateParametersChange();
}
