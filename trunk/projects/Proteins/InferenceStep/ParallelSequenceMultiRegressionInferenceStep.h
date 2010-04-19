/*-----------------------------------------.---------------------------------.
| Filename: ParallelSequenceMultiRegres...h| Base class for                  |
| Author  : Francis Maes                   |   ScoreVectorSequence regression|
| Started : 19/04/2010 17:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_PARALLEL_SEQUENCE_MULTI_REGRESSION_H_
# define LBCPP_INFERENCE_STEP_PARALLEL_SEQUENCE_MULTI_REGRESSION_H_

# include "ParallelInferenceStep.h"
# include "RegressionInferenceStep.h"
# include "../InferenceData/ScoreVectorSequence.h"

namespace lbcpp
{

// Input: a sequence of features
// Output/Supervision: a ScoreVectorSequence
// SubInference: FeaturesContainer -> DenseVector
class ParallelSequenceMultiRegressionInferenceStep : public SharedParallelInferenceStep
{
public:
  ParallelSequenceMultiRegressionInferenceStep(const String& name, InferenceStepPtr subInference)
    : SharedParallelInferenceStep(name, subInference) {}
  ParallelSequenceMultiRegressionInferenceStep()
    {}

  virtual size_t getNumSubInferences(ObjectPtr input) const
  {
    ObjectContainerPtr container = input.dynamicCast<ObjectContainer>();
    jassert(container);
    return container->size();
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index) const
  {
    if (!supervision)
      return ObjectPtr();
    ScoreVectorSequencePtr sequence = supervision.dynamicCast<ScoreVectorSequence>();
    jassert(sequence);
    return sequence->get(index);
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ScoreVectorSequencePtr sequence = output.dynamicCast<ScoreVectorSequence>();
    jassert(sequence);
    DenseVectorPtr vector = subOutput.dynamicCast<DenseVector>();
    jassert(vector);
    sequence->set(index, vector); 
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_PARALLEL_SEQUENCE_MULTI_REGRESSION_H_
