/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInference.cpp         | Decorator Inference             |
| Author  : Francis Maes                   |   base classes                  |
| Started : 26/11/2010 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** DecoratorInference
*/
DecoratorInference::DecoratorInference(const String& name)
  : Inference(name)
{
  setBatchLearner(decoratorInferenceLearner());
}

Variable DecoratorInference::computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
{
  DecoratorInferenceStatePtr state = prepareInference(context, input, supervision);
  if (!state)
    return Variable();

  const InferencePtr& subInference = state->getSubInference();
  if (subInference)
  {
    Variable subOutput;
    if (!subInference->run(context, state->getSubInput(), state->getSubSupervision(), subOutput))
      return Variable();
    state->setSubOutput(subOutput);
  }

  return finalizeInference(context, state);
}

/*
** StaticDecoratorInference
*/
String StaticDecoratorInference::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

void StaticDecoratorInference::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  DecoratorInference::clone(context, target);
  if (decorated)
    target.staticCast<StaticDecoratorInference>()->decorated = decorated->cloneAndCast<Inference>(context);
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

Variable PostProcessInference::finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState) const
  {return postProcessingFunction->computeFunction(context, Variable::pair(finalState->getInput(), finalState->getSubOutput()));}
