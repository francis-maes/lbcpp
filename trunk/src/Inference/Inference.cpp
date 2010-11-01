/*-----------------------------------------.---------------------------------.
| Filename: Inference.cpp                  | Inference step base class       |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Pair.h>
#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Inference/InferenceStack.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/InferenceOnlineLearner.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** Inference
*/
Inference::~Inference()
{
  while (onlineLearner)
  {
    onlineLearner->setPreviousLearner(InferenceOnlineLearnerPtr());
    onlineLearner = onlineLearner->getNextLearner();
  }
}

String Inference::getDescription(const Variable& input, const Variable& supervision) const
  {return getClassName() + T("(") + input.toShortString() + T(", ") + supervision.toShortString() + T(")");}

void Inference::clone(const ObjectPtr& t) const
{
  const InferencePtr& target = t.staticCast<Inference>();
  NameableObject::clone(target);
  if (onlineLearner)
  {
    target->onlineLearner = onlineLearner->cloneAndCast<InferenceOnlineLearner>();
    jassert(!target->onlineLearner->getNextLearner() || target->onlineLearner->getNextLearner()->getPreviousLearner() == target->onlineLearner);
  }
  ScopedReadLock _(parametersLock);
  if (parameters.exists())
  {
    target->parameters = getParametersCopy(); // clone parameters
    target->parametersChangedCallback();
  }
}

Variable Inference::getParameters() const
{
  ScopedReadLock _(parametersLock);
  return parameters;
}

Variable Inference::getParametersCopy() const
{
  ScopedReadLock _(parametersLock);
  return parameters.isObject() ? Variable(parameters.getObject()->clone()) : parameters;
}

void Inference::setParameters(const Variable& parameters)
{
  if (checkInheritance(parameters, getParametersType()))
  {
    ScopedWriteLock _(parametersLock);
    this->parameters = parameters;
  }
  parametersChangedCallback();
}

void Inference::setBatchLearner(InferencePtr batchLearner)
{
  if (batchLearner->getName() == T("Unnamed"))
    batchLearner->setName(getName() + T(" learner"));
  this->batchLearner = batchLearner;
}

void Inference::addOnlineLearner(const InferenceOnlineLearnerPtr& learner, bool insertInFront)
{
  InferenceOnlineLearnerPtr currentLearner = this->onlineLearner;
  if (!currentLearner)
    this->onlineLearner = learner;
  else if (insertInFront)
  {
    this->onlineLearner = learner;
    if (currentLearner)
      learner->setNextLearner(currentLearner);
  }
  else
    currentLearner->getLastLearner()->setNextLearner(learner);
}

const InferenceOnlineLearnerPtr& Inference::getOnlineLearner() const
  {return onlineLearner;}

InferenceOnlineLearnerPtr Inference::getLastOnlineLearner() const
  {return onlineLearner->getLastLearner();}

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

void StaticDecoratorInference::clone(const ObjectPtr& target) const
{
  DecoratorInference::clone(target);
  if (decorated)
    target.staticCast<StaticDecoratorInference>()->decorated = decorated->cloneAndCast<Inference>();
}

/*
** PostProcessInference
*/
PostProcessInference::PostProcessInference(InferencePtr decorated, FunctionPtr postProcessingFunction)
  : StaticDecoratorInference(postProcessingFunction->toString() + T("(") + decorated->getName() + T(")"), decorated),
    postProcessingFunction(postProcessingFunction)
{
  setBatchLearner(postProcessInferenceLearner());
}

TypePtr PostProcessInference::getOutputType(TypePtr inputType) const
  {return postProcessingFunction->getOutputType(pairClass(inputType, decorated->getOutputType(inputType)));}

Variable PostProcessInference::finalizeInference(InferenceContextWeakPtr context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
  {return postProcessingFunction->compute(Variable::pair(finalState->getInput(), finalState->getSubOutput()));}

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

void VectorParallelInference::clone(const ObjectPtr& t) const
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

SequentialInferenceStatePtr VectorSequentialInference::prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
  if (subInferences.size())
    prepareSubInference(context, state, 0, returnCode);
  return state;
}

bool VectorSequentialInference::updateInference(InferenceContextWeakPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
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

void VectorSequentialInference::clone(const ObjectPtr& t) const
{
  ReferenceCountedObjectPtr<VectorSequentialInference> target = t.staticCast<VectorSequentialInference>();
  StaticSequentialInference::clone(target);
  jassert(target->subInferences.size() == subInferences.size());
  for (size_t i = 0; i < subInferences.size(); ++i)
    target->subInferences[i] = subInferences[i]->cloneAndCast<Inference>();
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
  GetInferencesThatHaveAnOnlineLearnerVisitor(std::vector<InferencePtr>& res)
    : res(res) {}

  std::vector<InferencePtr>& res;

  virtual void visit(InferencePtr inference)
  {
    accept(inference);
    if (inference->getOnlineLearner())
      res.push_back(inference);
  }
};

void Inference::getInferencesThatHaveAnOnlineLearner(std::vector<InferencePtr>& res) const
{
  GetInferencesThatHaveAnOnlineLearnerVisitor visitor(res);
  visitor.visit(refCountedPointerFromThis(this));
}

/*
** InferenceBatchLearnerInput
*/
static void convert(const ContainerPtr& examples, std::vector< std::pair<Variable, Variable> >& res)
{
  if (examples)
  {
    size_t n = examples->getNumElements();
    res.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr pair = examples->getElement(i).getObjectAndCast<Pair>();
      res[i].first = pair->getFirst();
      res[i].second = pair->getSecond();
    }
  }
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, const ContainerPtr& trainingExamples, const ContainerPtr& validationExamples)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference)
{
  convert(trainingExamples, trainingData);
  convert(validationExamples, validationData);
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, size_t numTrainingExamples, size_t numValidationExamples)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference),
    trainingData(numTrainingExamples), validationData(numValidationExamples)
{
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, const ExampleVector& trainingData, const ExampleVector& validationData)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference),
    trainingData(trainingData), validationData(validationData)
{
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference)
{
}
