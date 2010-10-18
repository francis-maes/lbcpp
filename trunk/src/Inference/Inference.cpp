/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParameterizedInference.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
using namespace lbcpp;

/*
** Inference
*/
String Inference::getDescription(const Variable& input, const Variable& supervision) const
  {return getClassName() + T("(") + input.toShortString() + T(", ") + supervision.toShortString() + T(")");}

void Inference::clone(ObjectPtr target) const
{
  NameableObject::clone(target);
  if (onlineLearner)
    InferencePtr(target)->onlineLearner = onlineLearner->cloneAndCast<InferenceOnlineLearner>();
}

void Inference::setBatchLearner(InferencePtr batchLearner)
{
  if (batchLearner->getName() == T("Unnamed"))
    batchLearner->setName(getName() + T(" learner"));
  this->batchLearner = batchLearner;
}

/*
** DecoratorInference
*/
DecoratorInference::DecoratorInference(const String& name)
  : Inference(name)
{
  setBatchLearner(decoratorInferenceLearner());
}

String StaticDecoratorInference::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

/*
** ParallelInference
*/
StaticParallelInference::StaticParallelInference(const String& name)
  : ParallelInference(name)
{
  setBatchLearner(staticParallelInferenceLearner());
}

SharedParallelInference::SharedParallelInference(const String& name, InferencePtr subInference)
  : StaticParallelInference(name), subInference(subInference)
{
  setBatchLearner(sharedParallelInferenceLearner());
}

Variable SharedParallelInference::run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
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

void VectorParallelInference::clone(ObjectPtr t) const
{
  ReferenceCountedObjectPtr<VectorParallelInference> target = t.staticCast<VectorParallelInference>();
  StaticParallelInference::clone(target);
  jassert(target->subInferences.size() == subInferences.size());
  for (size_t i = 0; i < subInferences.size(); ++i)
    target->subInferences[i] = subInferences[i]->cloneAndCast<Inference>();
}

/*
** SequentialInference
*/
StaticSequentialInference::StaticSequentialInference(const String& name)
  : SequentialInference(name)
{
  setBatchLearner(staticSequentialInferenceLearner());
}

VectorSequentialInference::VectorSequentialInference(const String& name)
  : StaticSequentialInference(name) {}

SequentialInferenceStatePtr VectorSequentialInference::prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
  if (subInferences.size())
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
  if (index < (int)subInferences.size())
  {
    prepareSubInference(context, state, (size_t)index, returnCode);
    return true;
  }
  else
    return false;
}

void VectorSequentialInference::clone(ObjectPtr t) const
{
  ReferenceCountedObjectPtr<VectorSequentialInference> target = t.staticCast<VectorSequentialInference>();
  StaticSequentialInference::clone(target);
  jassert(target->subInferences.size() == subInferences.size());
  for (size_t i = 0; i < subInferences.size(); ++i)
    target->subInferences[i] = subInferences[i]->cloneAndCast<Inference>();
}

/*
** ParameterizedInference
*/
void ParameterizedInference::clone(ObjectPtr t) const
{
  const ParameterizedInferencePtr& target = t.staticCast<ParameterizedInference>();
  ScopedReadLock _(parametersLock);
  Inference::clone(target);
  if (parameters)
  {
    target->parameters = parameters->deepClone();
    target->parametersChangedCallback();
  }
}

ObjectPtr ParameterizedInference::getParameters() const
{
  ScopedReadLock _(parametersLock);
  return parameters ? parameters->deepClone() : ObjectPtr();
}

void ParameterizedInference::setParameters(ObjectPtr parameters)
{
  {
    ScopedWriteLock _(parametersLock);
    this->parameters = parameters;
  }
  parametersChangedCallback();
}

/*
** InferenceVisitor
*/
class InferenceVisitor
{
public:
  virtual ~InferenceVisitor() {}

  virtual void visit(InferencePtr inference)
    {accept(inference);}

protected:
  void accept(InferencePtr inference)
  {
    size_t n = inference->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      if (inference->getVariableType(i)->inheritsFrom(inferenceClass))
      {
        InferencePtr subInference = inference->getVariable(i).getObjectAndCast<Inference>();
        if (subInference)
          visit(subInference);
      }
      else if (inference->getVariableType(i)->inheritsFrom(containerClass(inferenceClass)))
      {
        ContainerPtr subInferences = inference->getVariable(i).getObjectAndCast<Container>();
        size_t m = subInferences->getNumElements();
        for (size_t j = 0; j < m; ++j)
        {
          InferencePtr subInference = subInferences->getElement(j).getObjectAndCast<Inference>();
          if (subInference)
            visit(subInference);
        }
      }
    }
  }
};

class GetInferencesThatHaveAnOnlineLearnerVisitor : public InferenceVisitor
{
public:
  GetInferencesThatHaveAnOnlineLearnerVisitor(std::set<InferencePtr>& res)
    : res(res) {}

  std::set<InferencePtr>& res;

  virtual void visit(InferencePtr inference)
  {
    if (inference->getOnlineLearner())
      res.insert(inference);
    accept(inference);
  }
};

void Inference::getInferencesThatHaveAnOnlineLearner(std::set<InferencePtr>& res) const
{
  GetInferencesThatHaveAnOnlineLearnerVisitor visitor(res);
  visitor.visit(refCountedPointerFromThis(this));
}
