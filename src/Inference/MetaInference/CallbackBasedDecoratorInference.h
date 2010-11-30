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
  CallbackBasedDecoratorInference(const String& name, InferencePtr decorated, ExecutionCallbackPtr callback)
    : StaticDecoratorInference(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInference() {}
  
  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    context.appendCallback(callback);
    return StaticDecoratorInference::prepareInference(context, input, supervision);
  }

  virtual Variable finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState) const
  {
    Variable res = StaticDecoratorInference::finalizeInference(context, finalState);
    context.removeCallback(callback);
    return res;
  }

protected:
  ExecutionCallbackPtr callback;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_BASED_DECORATOR_H_
