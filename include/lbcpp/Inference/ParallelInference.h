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

class ParallelInferenceState : public Object
{
public:
  ParallelInferenceState(ObjectPtr input, ObjectPtr supervision)
    : input(input), supervision(supervision) {}

  void reserve(size_t size)
    {subInferences.reserve(size);}

  ObjectPtr getInput() const
    {return input;}

  ObjectPtr getSupervision() const
    {return supervision;}

  size_t getNumSubInferences() const
    {return subInferences.size();}

  void addSubInference(InferencePtr subInference, ObjectPtr subInput, ObjectPtr subSupervision)
    {subInferences.push_back(SubInference(subInference, subInput, subSupervision));}

  InferencePtr getSubInference(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].inference;}

  ObjectPtr getSubInput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].input;}

  ObjectPtr getSubSupervision(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].supervision;}

  ObjectPtr getSubOutput(size_t index) const
    {jassert(index < subInferences.size()); return subInferences[index].output;}

  void setSubOutput(size_t index, ObjectPtr subOutput)
    {jassert(index < subInferences.size()); subInferences[index].output = subOutput;}

private:
  ObjectPtr input;
  ObjectPtr supervision;

  struct SubInference
  {
    SubInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision)
      : inference(inference), input(input), supervision(supervision) {}

    InferencePtr inference;
    ObjectPtr input;
    ObjectPtr supervision;
    ObjectPtr output;
  };

  std::vector<SubInference> subInferences;
};

typedef ReferenceCountedObjectPtr<ParallelInferenceState> ParallelInferenceStatePtr;

class ParallelInference : public Inference
{
public:
  ParallelInference(const String& name) : Inference(name) {}
  ParallelInference() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInference(ParallelInferencePtr(this), input, supervision, returnCode);}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;
  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode) = 0;
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
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);

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
