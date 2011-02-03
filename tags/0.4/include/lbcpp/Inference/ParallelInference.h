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
# include "InferenceCallback.h"
# include "../Core/Vector.h"

namespace lbcpp
{

class ParallelInferenceState : public InferenceState
{
public:
  ParallelInferenceState(const Variable& input, const Variable& supervision)
    : InferenceState(input, supervision) {}

  void reserve(size_t size)
    {subInferences.reserve(size);}

  size_t getNumSubInferences() const
    {return subInferences.size();}

  void addSubInference(const InferencePtr& subInference, const Variable& subInput, const Variable& subSupervision)
    {subInferences.push_back(SubInference(subInference, subInput, subSupervision));}

  const InferencePtr& getSubInference(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].inference;}

  const Variable& getSubInput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].input;}

  const Variable& getSubSupervision(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].supervision;}

  const Variable& getSubOutput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].output;}
  
  Variable& getSubOutput(size_t index)
    {jassert(index < subInferences.size()); return subInferences[index].output;}

  void setSubOutput(size_t index, const Variable& subOutput)
    {jassert(index < subInferences.size()); subInferences[index].output = subOutput;}

  lbcpp_UseDebuggingNewOperator

private:
  struct SubInference
  {
    SubInference(const InferencePtr& inference, const Variable& input, const Variable& supervision)
      : inference(inference), input(input), supervision(supervision) {}

    InferencePtr inference;
    Variable input;
    Variable supervision;
    Variable output;
  };

  std::vector<SubInference> subInferences;
};

typedef ReferenceCountedObjectPtr<ParallelInferenceState> ParallelInferenceStatePtr;

class ParallelInference : public Inference
{
public:
  ParallelInference(const String& name) : Inference(name), pushChildrenIntoStack(false) {}
  ParallelInference() {}

  virtual bool useMultiThreading() const
    {return true;}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const = 0;
  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const = 0;

  // push into stack
  void setPushChildrenIntoStackFlag(bool value)
    {pushChildrenIntoStack = value;}

  bool hasPushChildrenIntoStackFlag() const
    {return pushChildrenIntoStack;}

  virtual String getProgressionUnit() const
    {return T("Inferences");}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ParallelInferenceClass;

  bool pushChildrenIntoStack;
};

extern ClassPtr parallelInferenceClass;

class StaticParallelInference : public ParallelInference
{
public:
  StaticParallelInference(const String& name);
  StaticParallelInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;
};

extern ClassPtr staticParallelInferenceClass;

class VectorParallelInference : public StaticParallelInference
{
public:
  VectorParallelInference(const String& name)
    : StaticParallelInference(name) {}
  VectorParallelInference() {}

  virtual size_t getNumSubInferences() const
    {return subInferences.size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences[index];}
 
  void setSubInference(size_t index, InferencePtr inference)
    {subInferences[index] = inference;}
 
  void appendInference(InferencePtr inference)
    {subInferences.push_back(inference);}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class VectorParallelInferenceClass;

  std::vector<InferencePtr> subInferences;
};

extern ClassPtr vectorParallelInferenceClass;

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
  ** Object
  */
  virtual String toString() const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    StaticParallelInference::clone(context, target);
    if (subInference)
      SharedParallelInferencePtr(target)->subInference = subInference->cloneAndCast<Inference>(context);
  }

protected:
  friend class SharedParallelInferenceClass;

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const;

  InferencePtr subInference;
};

extern ClassPtr sharedParallelInferenceClass;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARALLEL_H_
