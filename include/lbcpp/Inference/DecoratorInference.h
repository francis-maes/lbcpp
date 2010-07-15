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

class DecoratorInferenceState : public InferenceState
{
public:
  DecoratorInferenceState(const Variable& input, const Variable& supervision)
    : InferenceState(input, supervision) {}

  void setSubInference(InferencePtr subInference, const Variable& subInput, const Variable& subSupervision)
  {
    this->subInference = subInference;
    this->subInput = subInput;
    this->subSupervision = subSupervision;
  }

  InferencePtr getSubInference() const
    {return subInference;}

  Variable getSubInput() const
    {return subInput;}

  Variable getSubSupervision() const
    {return subSupervision;}

  void setSubOutput(const Variable& subOutput)
    {this->subOutput = subOutput;}

  Variable getSubOutput() const
    {return subOutput;}

protected:
  Variable subInput;
  Variable subSupervision;
  Variable subOutput;
  InferencePtr subInference;
};

typedef ReferenceCountedObjectPtr<DecoratorInferenceState> DecoratorInferenceStatePtr;

class DecoratorInference : public Inference
{
public:
  DecoratorInference(const String& name);
  DecoratorInference() {}
 
  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
    {return finalState->getSubOutput();}

  /*
  ** Inference
  */
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return context->runDecoratorInference(DecoratorInferencePtr(this), input, supervision, returnCode);}
};

extern ClassPtr decoratorInferenceClass();

class StaticDecoratorInference : public DecoratorInference
{
public:
  StaticDecoratorInference(const String& name, InferencePtr decorated)
    : DecoratorInference(name), decorated(decorated) {}
  StaticDecoratorInference() {}

  virtual TypePtr getInputType() const
    {return decorated->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return decorated->getSupervisionType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return decorated->getOutputType(inputType);}

  virtual void beginRunSession()
    {decorated->beginRunSession();}

  virtual void endRunSession()
    {decorated->endRunSession();}

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, input, supervision);
    return res;
  }

  InferencePtr getSubInference() const
    {return decorated;}

  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;
  virtual void clone(ObjectPtr target) const;
  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const;
 
protected:
  InferencePtr decorated;
};

typedef ReferenceCountedObjectPtr<StaticDecoratorInference> StaticDecoratorInferencePtr;

extern ClassPtr staticDecoratorInferenceClass();

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_H_
