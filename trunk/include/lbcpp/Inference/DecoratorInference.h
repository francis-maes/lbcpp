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
# include "InferenceCallback.h"
# include "../Function/Function.h"

namespace lbcpp
{

class DecoratorInferenceState : public InferenceState
{
public:
  DecoratorInferenceState(const Variable& input, const Variable& supervision)
    : InferenceState(input, supervision) {}

  void setSubInference(const InferencePtr& subInference, const Variable& subInput, const Variable& subSupervision)
  {
    this->subInference = subInference;
    this->subInput = subInput;
    this->subSupervision = subSupervision;
  }

  const InferencePtr& getSubInference() const
    {return subInference;}

  const Variable& getSubInput() const
    {return subInput;}

  const Variable& getSubSupervision() const
    {return subSupervision;}

  void setSubOutput(const Variable& subOutput)
    {this->subOutput = subOutput;}

  const Variable& getSubOutput() const
    {return subOutput;}

  lbcpp_UseDebuggingNewOperator

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
 
  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const = 0;
  virtual Variable finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState) const
    {return finalState->getSubOutput();}

  /*
  ** Inference
  */
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr decoratorInferenceClass;

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

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    res->setSubInference(decorated, input, supervision);
    return res;
  }

  const InferencePtr& getSubInference() const
    {return decorated;}

  void setSubInference(const InferencePtr& subInference)
    {decorated = subInference;}

  /*
  ** Object
  */
  virtual String toString() const;
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class StaticDecoratorInferenceClass;

  InferencePtr decorated;
};

typedef ReferenceCountedObjectPtr<StaticDecoratorInference> StaticDecoratorInferencePtr;

extern ClassPtr staticDecoratorInferenceClass;

class PostProcessInference : public StaticDecoratorInference
{
public:
  PostProcessInference(InferencePtr decorated, FunctionPtr postProcessingFunction);
  PostProcessInference() {}

  virtual TypePtr getOutputType(TypePtr inputType) const;
  virtual Variable finalizeInference(ExecutionContext& context, const DecoratorInferenceStatePtr& finalState) const;

protected:
  friend class PostProcessInferenceClass;
  FunctionPtr postProcessingFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_H_
