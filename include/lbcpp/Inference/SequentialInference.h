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
    : InferenceState(input, supervision), stepNumber(0) {}

  const Variable& getCurrentObject() const
    {return currentObject;}

  void setCurrentObject(const Variable& object)
    {currentObject = object;}

  size_t getCurrentStepNumber() const
    {return stepNumber;}

  void incrementStepNumber()
    {++stepNumber;}

  InferencePtr getCurrentSubInference() const
    {return subInference;}

  void setCurrentSubInference(InferencePtr subInference)
    {this->subInference = subInference;}

  bool isFinal() const
    {return !subInference;}

private:
  Variable currentObject;
  size_t stepNumber;
  InferencePtr subInference;
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
  virtual ObjectPtr prepareInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return state->getInput();}

  virtual InferencePtr getInitialSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const = 0;

  virtual std::pair<Variable, Variable> prepareSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return std::make_pair(state->getCurrentObject(), state->getSupervision());}
  
  virtual Variable finalizeSubInference(SequentialInferenceStatePtr state, const Variable& subInferenceOutput, ReturnCode& returnCode) const
    {return subInferenceOutput;}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const = 0;

  virtual Variable finalizeInference(SequentialInferenceStatePtr finalState, ReturnCode& returnCode) const
    {return finalState->getCurrentObject();}

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

class StaticSequentialInference : public SequentialInference
{
public:
  StaticSequentialInference(const String& name)
    : SequentialInference(name) {}
  StaticSequentialInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const;
};

class VectorSequentialInference : public StaticSequentialInference
{
public:
  VectorSequentialInference(const String& name)
    : StaticSequentialInference(name) {}
  VectorSequentialInference() {}

  virtual InferencePtr getInitialSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return subInferences.get(0);}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
  {
    size_t index = state->getCurrentStepNumber();
    jassert(state->getCurrentSubInference() == subInferences.get(index));
    ++index;
    return index < subInferences.size() ? subInferences.get(index) : InferencePtr();
  }

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
