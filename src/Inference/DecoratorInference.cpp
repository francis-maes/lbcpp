/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInference.cpp         | Decorator Inference             |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 22:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/DecoratorInference.h>
using namespace lbcpp;

/*
** DecoratorInference
*/
DecoratorInference::DecoratorInference(const String& name)
  : Inference(name)
  {setBatchLearner(decoratorInferenceLearner());}

String StaticDecoratorInference::toString() const
  {return getClassName() + T("(") + (decorated ? decorated->toString() : T("<null>")) + T(")");}

class PostProcessInference : public StaticDecoratorInference
{
public:
  // postProcessingFunction: from (object,any) pair to object
  PostProcessInference(InferencePtr decorated, FunctionPtr postProcessingFunction)
    : StaticDecoratorInference(postProcessingFunction->toString() + T("(") + decorated->getName() + T(")"), decorated),
        postProcessingFunction(postProcessingFunction)
    {setBatchLearner(postProcessInferenceLearner());}

  PostProcessInference() {}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return postProcessingFunction->getOutputType(pairType(inputType, decorated->getOutputType(inputType)));}

  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
    {return postProcessingFunction->compute(Variable::pair(finalState->getInput(), finalState->getSubOutput()));}

protected:
  FunctionPtr postProcessingFunction;
};

InferencePtr lbcpp::postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction)
  {return new PostProcessInference(inference, postProcessingFunction);}

void declareDecoratorInferenceClasses()
{
  LBCPP_DECLARE_CLASS(PostProcessInference, DecoratorInference);
};
