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
# include "../InferenceCallback/InferenceContext.h"

namespace lbcpp
{

class ParallelInferenceStep : public InferenceStep
{
public:
  ParallelInferenceStep(const String& name) : InferenceStep(name) {}

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

  virtual InferenceStepPtr getSubInference(ObjectPtr input, size_t index) const
    {return subInference;}

protected:
  InferenceStepPtr subInference;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_PARALLEL_H_
