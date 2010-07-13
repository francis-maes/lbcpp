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
# include "../Data/Function.h"

namespace lbcpp
{

class DecoratorInference : public Inference
{
public:
  DecoratorInference(const String& name, InferencePtr decorated)
    : Inference(name), decorated(decorated) {}
  DecoratorInference() {}
 
  virtual std::pair<Variable, Variable> prepareSubInference(const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return std::make_pair(input, supervision);}
    
  virtual Variable finalizeSubInference(const Variable& input, const Variable& supervision, const Variable& subInferenceOutput, ReturnCode& returnCode) const
    {return subInferenceOutput;}
 
  /*
  ** Inference
  */
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
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

class PostProcessInference : public DecoratorInference
{
public:
  // postProcessingFunction: from (object,any) pair to object
  PostProcessInference(InferencePtr decorated, FunctionPtr postProcessingFunction)
    : DecoratorInference(postProcessingFunction->toString() + T("(") + decorated->getName() + T(")"), decorated),
        postProcessingFunction(postProcessingFunction) {}
  PostProcessInference() {}
  
  virtual Variable finalizeSubInference(const Variable& input, const Variable& supervision, const Variable& subInferenceOutput, ReturnCode& returnCode) const
    {return postProcessingFunction->compute(Variable::pair(input, subInferenceOutput));}

protected:
  FunctionPtr postProcessingFunction;
};

inline InferencePtr postProcessInference(InferencePtr inference, FunctionPtr postProcessingFunction)
  {return new PostProcessInference(inference, postProcessingFunction);}

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_H_
