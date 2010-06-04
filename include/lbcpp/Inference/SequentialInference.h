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
# include "InferenceVisitor.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../Object/ObjectPair.h"

namespace lbcpp
{

class SequentialInferenceState : public Object
{
public:
  SequentialInferenceState(ObjectPtr input, ObjectPtr supervision)
    : input(input), supervision(supervision), stepNumber(0) {}

  ObjectPtr getInput() const
    {return input;}

  ObjectPtr getSupervision() const
    {return supervision;}

  ObjectPtr getCurrentObject() const
    {return currentObject;}

  void setCurrentObject(ObjectPtr object)
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
  ObjectPtr input;
  ObjectPtr supervision;
  ObjectPtr currentObject;
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

  virtual ObjectPairPtr prepareSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return new ObjectPair(state->getCurrentObject(), state->getSupervision());}
  
  virtual ObjectPtr finalizeSubInference(SequentialInferenceStatePtr state, ObjectPtr subInferenceOutput, ReturnCode& returnCode) const
    {return subInferenceOutput;}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const = 0;

  virtual ObjectPtr finalizeInference(SequentialInferenceStatePtr finalState, ReturnCode& returnCode) const
    {return finalState->getCurrentObject();}

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SequentialInferencePtr(this));}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runSequentialInference(SequentialInferencePtr(this), input, supervision, returnCode);}

  /*
  ** Object
  */
  virtual String toString() const;
};

class VectorSequentialInference : public SequentialInference
{
public:
  VectorSequentialInference(const String& name)
    : SequentialInference(name) {}

  virtual InferencePtr getInitialSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
    {return subInferences.get(0);}

  virtual InferencePtr getNextSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
  {
    size_t index = state->getCurrentStepNumber();
    jassert(state->getCurrentSubInference() == subInferences.get(index));
    ++index;
    return index < subInferences.size() ? subInferences.get(index) : InferencePtr();
  }

  size_t getNumSubInferences() const
    {return subInferences.size();}

  InferencePtr getSubInference(size_t index) const
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
