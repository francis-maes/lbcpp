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
    {jassert(index < subInferences.size()); subInferences[index].output = subOutput;}

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

class StaticParallelInference : public ParallelInference
{
public:
  StaticParallelInference(const String& name)
    : ParallelInference(name) {}
  StaticParallelInference() {}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;

  virtual void getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& subObjects) const;
};

class VectorStaticParallelInference : public StaticParallelInference
{
public:
  VectorStaticParallelInference(const String& name)
    : StaticParallelInference(name) {}
  VectorStaticParallelInference() {}

  virtual size_t getNumSubInferences() const
    {return subInferences.size();}

  virtual InferencePtr getSubInference(size_t index) const
    {return subInferences.get(index);}
 
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInferences.saveToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && subInferences.loadFromDirectory(file);}

protected:
  InferenceVector subInferences;
};

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
  virtual bool loadFromFile(const File& file);
  virtual bool saveToFile(const File& file) const;

protected:
  InferencePtr subInference;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARALLEL_H_
