/*-----------------------------------------.---------------------------------.
| Filename: ParallelSequenceLabelingInfe..h| Base class for sequence labeling|
| Author  : Francis Maes                   |   parallel steps                |
| Started : 16/04/2010 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_SEQUENCE_LABELING_PARALLEL_H_
# define LBCPP_INFERENCE_STEP_SEQUENCE_LABELING_PARALLEL_H_

# include "ParallelInferenceStep.h"
# include "ClassificationInferenceStep.h"

namespace lbcpp
{

class ParallelSequenceLabelingInferenceStep : public SharedParallelInferenceStep
{
public:
  ParallelSequenceLabelingInferenceStep(const String& name)
    : SharedParallelInferenceStep(name, new ClassificationInferenceStep(name + T("Classification"))) {}
  ParallelSequenceLabelingInferenceStep()
    {}

  virtual FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t index) const = 0;

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    return container->size();
  }

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return getInputFeatures(input, index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    ObjectContainerPtr s = supervision.dynamicCast<ObjectContainer>();
    jassert(s);
    return s->get(index);
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {output.dynamicCast<ObjectContainer>()->set(index, subOutput);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_SEQUENCE_LABELING_PARALLEL_H_
