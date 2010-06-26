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

class CallbackBasedDecoratorInference : public DecoratorInference
{
public:
  CallbackBasedDecoratorInference(const String& name, InferencePtr decorated, InferenceCallbackPtr callback)
    : DecoratorInference(name, decorated), callback(callback) {}
  CallbackBasedDecoratorInference() {}
 
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    jassert(callback);
    context->appendCallback(callback);
    ObjectPtr res = DecoratorInference::run(context, input, supervision, returnCode);
    context->removeCallback(callback);
    return res;
  }

protected:
  InferenceCallbackPtr callback;
  
  virtual bool load(InputStream& istr)
    {return DecoratorInference::load(istr) && lbcpp::read(istr, callback);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInference::save(ostr); lbcpp::write(ostr, callback);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_BASED_DECORATOR_H_
