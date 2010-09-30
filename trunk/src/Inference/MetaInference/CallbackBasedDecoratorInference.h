/*-----------------------------------------.---------------------------------.
| Filename: CallbackBasedDecoratorInfer...h| Applies a callback to a given   |
| Author  : Francis Maes                   |  inference                      |
| Started : 05/05/2010 15:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_BASED_DECORATOR_H_
# define LBCPP_INFERENCE_CALLBACK_BASED_DECORATOR_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class CallbackBasedDecoratorInference : public StaticDecoratorInference
{
public:
  CallbackBasedDecoratorInference(const String& name, InferencePtr decorated, InferenceCallbackPtr callback)
    : StaticDecoratorInference(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInference() {}
  
  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    context->appendCallback(callback);
    return StaticDecoratorInference::prepareInference(context, input, supervision, returnCode);
  }

  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
  {
    Variable res = StaticDecoratorInference::finalizeInference(context, finalState, returnCode);
    context->removeCallback(callback);
    return res;
  }

protected:
  InferenceCallbackPtr callback;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_BASED_DECORATOR_H_
