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

  void setSubInference(InferencePtr subInference, const Variable& subInput, const Variable& subSupervision)
  {
    this->subInference = subInference;
    this->subInput = subInput;
    this->subSupervision = subSupervision;
    incrementStepNumber();
  }

  void setFinalState()
    {setSubInference(InferencePtr(), Variable(), Variable());}

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

  bool isFinal() const
    {return !subInference;}

  int getStepNumber() const
    {return stepNumber;}

  void incrementStepNumber()
    {++stepNumber;}

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
  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  // returns false if the final state is reached
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode) = 0;

  virtual Variable finalizeInference(InferenceContextPtr context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
    {return finalState->getSubOutput();}

protected:
  /*
  ** Inference
  */
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return context->runSequentialInference(SequentialInferencePtr(this), input, supervision, returnCode);}
};

extern ClassPtr sequentialInferenceClass();

class StaticSequentialInference : public SequentialInference
{
public:
  StaticSequentialInference(const String& name);
  StaticSequentialInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;
};

typedef ReferenceCountedObjectPtr<StaticSequentialInference> StaticSequentialInferencePtr;

extern ClassPtr staticSequentialInferenceClass();

class VectorSequentialInference : public StaticSequentialInference
{
public:
  VectorSequentialInference(const String& name);
  VectorSequentialInference() {}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode);

  virtual void finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {}

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(getSubInference(index), index == 0 ? state->getInput() : state->getSubOutput(), state->getSupervision());}

  virtual size_t getNumSubInferences() const
    {return subInferences->size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences->getVariable(index).getObjectAndCast<Inference>();}
  
  void setSubInference(size_t index, InferencePtr inference)
    {subInferences->setVariable(index, inference);}
 
  void appendInference(InferencePtr inference)
    {subInferences->append(inference);}

  virtual void clone(ObjectPtr target) const
  {
    StaticSequentialInference::clone(target);
    VectorSequentialInferencePtr(target)->subInferences = subInferences->cloneAndCast<Vector>();
  }

protected:
  friend class VectorSequentialInferenceClass;
  VectorPtr subInferences;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_SEQUENTIAL_H_
