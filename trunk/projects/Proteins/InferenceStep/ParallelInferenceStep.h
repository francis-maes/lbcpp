/*-----------------------------------------.---------------------------------.
| Filename: ParallelInferenceStep.h        | Base class for parallel         |
| Author  : Francis Maes                   |   inference                     |
| Started : 08/04/2010 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_PARALLEL_H_
# define LBCPP_INFERENCE_STEP_PARALLEL_H_

# include "InferenceStep.h"
# include "../InferenceContext/InferenceVisitor.h"
# include "../InferenceContext/InferenceContext.h"

namespace lbcpp
{

class ParallelInferenceStep : public InferenceStep
{
public:
  ParallelInferenceStep(const String& name) : InferenceStep(name) {}
  ParallelInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(ParallelInferenceStepPtr(this));}
  
  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
    {return context->runParallelInferences(ParallelInferenceStepPtr(this), input, supervision, returnCode);}

  virtual size_t getNumSubInferences(ObjectPtr input) const = 0;
  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const = 0;

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const = 0;
  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const = 0;
};

class SharedParallelInferenceStep : public ParallelInferenceStep
{
public:
  SharedParallelInferenceStep(const String& name, InferenceStepPtr subInference)
    : ParallelInferenceStep(name), subInference(subInference) {}
  SharedParallelInferenceStep() {}

  virtual void accept(InferenceVisitorPtr visitor)
    {visitor->visit(SharedParallelInferenceStepPtr(this));}
  
  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

  virtual String toString() const
    {jassert(subInference); return getClassName() + T("(") + subInference->toString() + T(")");}

  virtual bool loadFromFile(const File& file)
  {
    if (!loadFromDirectory(file))
      return false;
    subInference = createFromFileAndCast<InferenceStep>(file.getChildFile(T("shared.inference")));
    return subInference != InferenceStepPtr();
  }

  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && subInference->saveToFile(file.getChildFile(T("shared.inference")));}

  InferenceStepPtr getSharedInferenceStep() const
    {return subInference;}

protected:
  InferenceStepPtr subInference;
};

class VectorParallelInferenceStep : public ParallelInferenceStep, public VectorBasedInferenceHelper
{
public:
  VectorParallelInferenceStep(const String& name)
    : ParallelInferenceStep(name) {}
  VectorParallelInferenceStep() {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return VectorBasedInferenceHelper::getNumSubSteps();}

  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return VectorBasedInferenceHelper::getSubStep(index);}
 
protected:
  virtual bool saveToFile(const File& file) const
    {return saveToDirectory(file) && saveSubInferencesToDirectory(file);}

  virtual bool loadFromFile(const File& file)
    {return loadFromDirectory(file) && loadSubInferencesFromDirectory(file);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_PARALLEL_H_
