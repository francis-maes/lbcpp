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
# include "InferenceCallback.h"
# include <lbcpp/Data/Pair.h>
# include <lbcpp/Data/Vector.h>

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

  lbcpp_UseDebuggingNewOperator

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
  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;

  // returns false if the final state is reached
  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr state, ReturnCode& returnCode) = 0;

  virtual Variable finalizeInference(ExecutionContext& context, SequentialInferenceStatePtr finalState, ReturnCode& returnCode)
    {return finalState->getSubOutput();}

  lbcpp_UseDebuggingNewOperator

protected:
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
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

  virtual TypePtr getInputType() const
    {return subInferences.size() ? subInferences[0]->getInputType() : anyType;}

  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr state, ReturnCode& returnCode);

  virtual void finalizeSubInference(ExecutionContext& context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {}

  virtual void prepareSubInference(ExecutionContext& context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(subInferences[index], index == 0 ? state->getInput() : state->getSubOutput(), state->getSupervision());}

  virtual size_t getNumSubInferences() const
    {return subInferences.size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences[index];}
  
  void setSubInference(size_t index, const InferencePtr& inference)
    {subInferences[index] = inference;}
 
  void appendInference(const InferencePtr& inference)
    {subInferences.push_back(inference);}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  friend class VectorSequentialInferenceClass;
  std::vector<InferencePtr> subInferences;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_SEQUENTIAL_H_
