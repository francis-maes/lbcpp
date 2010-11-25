/*-----------------------------------------.---------------------------------.
| Filename: DummyInferenceLearner.h        | A batch learner that            |
| Author  : Francis Maes                   |  does nothing                   |
| Started : 14/07/2010 13:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_DUMMY_LEARNER_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_DUMMY_LEARNER_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class DummyInferenceLearner : public InferenceBatchLearner<Inference>
{
protected:
  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return Variable();}
};

class MultiPassInferenceLearner : public InferenceBatchLearner<VectorSequentialInference>
{
public:
  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual void prepareSubInference(ExecutionContext& context, SequentialInferenceStatePtr state, size_t index, ReturnCode& returnCode)
    {state->setSubInference(getSubInference(index), state->getInput(), Variable());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_DUMMY_LEARNER_H_
