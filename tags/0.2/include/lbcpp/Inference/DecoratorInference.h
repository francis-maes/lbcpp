/*-----------------------------------------.---------------------------------.
| Filename: DecoratorInference.h           | Decorator Inference             |
| Author  : Francis Maes                   |                                 |
| Started : 27/05/2010 21:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DECORATOR_H_
# define LBCPP_INFERENCE_DECORATOR_H_

# include "Inference.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"

namespace lbcpp
{

class DecoratorInference : public Inference
{
public:
  DecoratorInference(const String& name, InferencePtr decorated)
    : Inference(name), decorated(decorated) {}
  DecoratorInference() {}
 
  virtual std::pair<ObjectPtr, ObjectPtr> prepareSubInference(ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return std::make_pair(input, supervision);}
    
  virtual ObjectPtr finalizeSubInference(ObjectPtr input, ObjectPtr supervision, ObjectPtr subInferenceOutput, ReturnCode& returnCode) const
    {return subInferenceOutput;}
 
  /*
  ** Inference
  */
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runDecoratorInference(DecoratorInferencePtr(this), input, supervision, returnCode);}

  InferencePtr getSubInference() const
    {return decorated;}

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const;
 
  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;
  virtual ObjectPtr clone() const;

protected:
  InferencePtr decorated;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_H_
