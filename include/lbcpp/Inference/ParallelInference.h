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
# include "InferenceVisitor.h"
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

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ParallelInferencePtr(this));}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInference(ParallelInferencePtr(this), input, supervision, returnCode);}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {jassert(false); return ParallelInferenceStatePtr();}
  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {jassert(false); return ObjectPtr();}

  virtual size_t getNumSubInferences(ObjectPtr input) const = 0;
  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const = 0;
};

class SharedParallelInference : public ParallelInference
{
public:
  SharedParallelInference(const String& name, InferencePtr subInference);
  SharedParallelInference() {}

  InferencePtr getSharedInferenceStep() const
    {return subInference;}

  void setSharedInferenceStep(InferencePtr step)
    {subInference = step;}

  /*
  ** ParallelInference
  */
  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

  /*
  ** Inference
  */
  virtual void accept(InferenceVisitorPtr visitor);
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

class StaticParallelInference : public ParallelInference
{
public:
  StaticParallelInference(const String& name)
    : ParallelInference(name) {}
  StaticParallelInference() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(StaticParallelInferencePtr(this));}

  virtual size_t getNumSubInferences() const = 0;
  virtual InferencePtr getSubInference(size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getNumSubInferences();}

  virtual InferencePtr getSubInference(ObjectPtr input, size_t index) const
    {return getSubInference(index);}
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

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_PARALLEL_H_
