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
# include "../Object/ObjectPair.h"

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

  /*
  ** Object
  */
  virtual String toString() const;

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

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const;
};

typedef ReferenceCountedObjectPtr<StaticSequentialInference> StaticSequentialInferencePtr;

extern ClassPtr staticSequentialInferenceClass();

class VectorSequentialInference : public StaticSequentialInference
{
public:
  VectorSequentialInference(const String& name)
    : StaticSequentialInference(name) {}
  VectorSequentialInference() {}

  virtual SequentialInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
    if (subInferences.size())
      prepareSubInference(context, state, 0, returnCode);
    return state;
  }

  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
  {
    int index = state->getStepNumber(); 
    jassert(index >= 0);
    finalizeSubInference(context, state, (size_t)index, returnCode);
    jassert(state->getSubInference() == subInferences.get(index));
    ++index;
    if (index < (int)subInferences.size())
    {
      prepareSubInference(context, state, (size_t)index, returnCode);
      return true;
    }
    else
      return false;
  }

  virtual void finalizeSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {}

  virtual void prepareSubInference(InferenceContextPtr context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(subInferences.get(index), index == 0 ? state->getInput() : state->getSubOutput(), state->getSupervision());}

  virtual size_t getNumSubInferences() const
    {return subInferences.size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences.get(index);}
  
  void setSubInference(size_t index, InferencePtr inference)
    {subInferences.set(index, inference);}
 
  void appendInference(InferencePtr inference)
    {subInferences.append(inference);}

  File getSubInferenceFile(size_t index, const File& modelDirectory) const
    {return subInferences.getSubInferenceFile(index, modelDirectory);}

  bool loadSubInferencesFromDirectory(const File& file)
    {return subInferences.loadFromDirectory(file);}

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInferences.saveToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && subInferences.loadFromDirectory(file);}

protected:
  InferenceVector subInferences;
};

typedef ReferenceCountedObjectPtr<VectorSequentialInference> VectorSequentialInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_SEQUENTIAL_H_
