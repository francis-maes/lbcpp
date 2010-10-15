/*-----------------------------------------.---------------------------------.
| Filename: SequentialInference.h          | Sequential Inference base       |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/05/2010 21:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_SEQUENTIAL_H_
# define LBCPP_INFERENCE_SEQUENTIAL_H_

# include "Inference.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../Data/Vector.h"

namespace lbcpp
{

class SequentialInferenceState : public InferenceState
{
public:
  SequentialInferenceState(const Variable& input, const Variable& supervision)
    : InferenceState(input, supervision), stepNumber(-1) {}

  void setSubInference(const InferencePtr& subInference, const Variable& subInput, const Variable& subSupervision)
  {
    this->subInference = subInference;
    this->subInput = subInput;
    this->subSupervision = subSupervision;
    incrementStepNumber();
  }

  void setFinalState()
    {setSubInference(InferencePtr(), Variable(), Variable());}

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

  bool isFinal() const
    {return !subInference;}

  int getStepNumber() const
    {return stepNumber;}

  void incrementStepNumber()
    {++stepNumber;}

  juce_UseDebuggingNewOperator

private:
  int stepNumber;
  InferencePtr subInference;
  Variable subInput;
  Variable subSupervision;
  Variable subOutput;
};
typedef ReferenceCountedObjectPtr<SequentialInferenceState> SequentialInferenceStatePtr;

class SequentialInference : public Inference
{
public:
  SequentialInference(const String& name) : Inference(name) {}
  SequentialInference() {}

  /*
  ** Abstract
  */
  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  // returns false if the final state is reached
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode) = 0;

  virtual Variable finalizeInference(const InferenceContextPtr& context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
    {return finalState->getSubOutput();}

  juce_UseDebuggingNewOperator

protected:
  /*
  ** Inference
  */
  virtual Variable run(InferenceContext* context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return context->runSequentialInference(this, input, supervision, returnCode);}
};

extern ClassPtr sequentialInferenceClass;

class StaticSequentialInference : public SequentialInference
{
public:
  StaticSequentialInference(const String& name);
  StaticSequentialInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;
};

typedef ReferenceCountedObjectPtr<StaticSequentialInference> StaticSequentialInferencePtr;

extern ClassPtr staticSequentialInferenceClass;

class VectorSequentialInference : public StaticSequentialInference
{
public:
  VectorSequentialInference(const String& name);
  VectorSequentialInference() {}

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode);

  virtual void finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {}

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(getSubInference(index), index == 0 ? state->getInput() : state->getSubOutput(), state->getSupervision());}

  virtual size_t getNumSubInferences() const
    {return subInferences->getNumElements();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences->getElement(index).getObjectAndCast<Inference>();}
  
  void setSubInference(size_t index, InferencePtr inference)
    {subInferences->setElement(index, inference);}
 
  void appendInference(InferencePtr inference)
    {subInferences->append(inference);}

  virtual void clone(ObjectPtr target) const
  {
    StaticSequentialInference::clone(target);
    VectorSequentialInferencePtr(target)->subInferences = subInferences->cloneContent();
  }

protected:
  friend class VectorSequentialInferenceClass;
  VectorPtr subInferences;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_SEQUENTIAL_H_
