/*-----------------------------------------.---------------------------------.
| Filename: ParallelInference.h            | Parallel Inference base classes |
| Author  : Francis Maes                   |                                 |
| Started : 27/05/2010 21:08               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_PARALLEL_H_
# define LBCPP_INFERENCE_PARALLEL_H_

# include "Inference.h"
# include "InferenceContext.h"
# include "InferenceCallback.h"
# include "../Data/Vector.h"

namespace lbcpp
{

class ParallelInferenceState : public InferenceState
{
public:
  ParallelInferenceState(const Variable& input, const Variable& supervision)
    : InferenceState(input, supervision), numOutputsSet(0) {}

  void reserve(size_t size)
    {subInferences.reserve(size);}

  size_t getNumSubInferences() const
    {return subInferences.size();}

  void addSubInference(InferencePtr subInference, const Variable& subInput, const Variable& subSupervision)
    {subInferences.push_back(SubInference(subInference, subInput, subSupervision));}

  InferencePtr getSubInference(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].inference;}

  const Variable& getSubInput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].input;}

  const Variable& getSubSupervision(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].supervision;}

  const Variable& getSubOutput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].output;}

  void setSubOutput(size_t index, const Variable& subOutput)
    {jassert(index < subInferences.size()); subInferences[index].output = subOutput; ++numOutputsSet;}

  bool haveAllOutputsBeenSet() const
  {juce::DBG("haveAllOutputsBeenSet: " + String((int)numOutputsSet) + T(" / ") + String((int)subInferences.size()));
    jassert(numOutputsSet <= subInferences.size()); return numOutputsSet == subInferences.size();}

private:
  struct SubInference
  {
    SubInference(InferencePtr inference, const Variable& input, const Variable& supervision)
      : inference(inference), input(input), supervision(supervision) {}

    InferencePtr inference;
    Variable input;
    Variable supervision;
    Variable output;
  };

  std::vector<SubInference> subInferences;
  size_t volatile numOutputsSet;
};

typedef ReferenceCountedObjectPtr<ParallelInferenceState> ParallelInferenceStatePtr;

class ParallelInference : public Inference
{
public:
  ParallelInference(const String& name) : Inference(name) {}
  ParallelInference() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode) = 0;

protected:
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return context->runParallelInference(ParallelInferencePtr(this), input, supervision, returnCode);}
};

extern ClassPtr parallelInferenceClass();

class StaticParallelInference : public ParallelInference
{
public:
  StaticParallelInference(const String& name);
  StaticParallelInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;
};

extern ClassPtr staticParallelInferenceClass();

class VectorParallelInference : public StaticParallelInference
{
public:
  VectorParallelInference(const String& name)
    : StaticParallelInference(name), subInferences(vector(inferenceClass())) {}
  VectorParallelInference() {}

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
    StaticParallelInference::clone(target);
    VectorParallelInferencePtr(target)->subInferences = subInferences->cloneAndCast<Vector>();
  }

protected:
  friend class VectorParallelInferenceClass;

  VectorPtr subInferences;
};

extern ClassPtr vectorParallelInferenceClass();

class SharedParallelInference : public StaticParallelInference
{
public:
  SharedParallelInference(const String& name, InferencePtr subInference);
  SharedParallelInference() {}

  InferencePtr getSubInference() const
    {return subInference;}

  void setSubInference(InferencePtr step)
    {subInference = step;}

  /*
  ** StaticParallelInference
  */
  virtual size_t getNumSubInferences() const
    {return 1;}

  virtual InferencePtr getSubInference(size_t index) const
    {jassert(index == 0); return subInference;}

  /*
  ** Inference
  */
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  /*
  ** Object
  */
  virtual String toString() const;

  virtual void clone(ObjectPtr target) const
  {
    StaticParallelInference::clone(target);
    if (subInference)
      SharedParallelInferencePtr(target)->subInference = subInference->cloneAndCast<Inference>();
  }

protected:
  friend class SharedParallelInferenceClass;

  InferencePtr subInference;
};

extern ClassPtr sharedParallelInferenceClass();

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARALLEL_H_
