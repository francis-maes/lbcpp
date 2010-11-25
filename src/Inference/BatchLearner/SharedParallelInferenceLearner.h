/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelInferenceLearner.h|  A batch learner to learn      |
| Author  : Francis Maes                   |  shared parellel inferences     |
| Started : 14/07/2010 14:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_SHARED_PARALLEL_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_SHARED_PARALLEL_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>

namespace lbcpp
{

class SharedParallelInferenceLearner : public InferenceBatchLearner<DecoratorInference>
{
public:
  SharedParallelInferenceLearner(bool filterUnsupervisedExamples = true)
    : filterUnsupervisedExamples(filterUnsupervisedExamples) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return sharedParallelInferenceClass;}

  virtual DecoratorInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const SharedParallelInferencePtr& targetInference = learnerInput->getTargetInference().staticCast<SharedParallelInference>();

    DecoratorInferenceStatePtr res(new DecoratorInferenceState(input, supervision));
    InferencePtr targetSubInference = targetInference->getSubInference();
    InferencePtr subLearner = targetSubInference->getBatchLearner();
    if (subLearner)
    {
      InferenceBatchLearnerInputPtr subLearnerInput = computeSubLearnerInput(context, targetInference, learnerInput);
      if (subLearnerInput)
        res->setSubInference(subLearner, subLearnerInput, Variable());
    }
    return res;
  }

private:
  bool filterUnsupervisedExamples;

  InferenceBatchLearnerInputPtr computeSubLearnerInput(ExecutionContext& context, const SharedParallelInferencePtr& targetInference, const InferenceBatchLearnerInputPtr& examples) const
  {
    const InferencePtr& targetSubInference = targetInference->getSubInference();

    InferenceBatchLearnerInputPtr res = new InferenceBatchLearnerInput(targetSubInference);
    
    size_t n = examples->getNumExamples();
    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = examples->getExample(i);
      bool isValidationExample = i >= examples->getNumTrainingExamples();

      ParallelInferenceStatePtr state = targetInference->prepareInference(context, example.first, example.second);
      if (!state)
        return InferenceBatchLearnerInputPtr();

      for (size_t j = 0; j < state->getNumSubInferences(); ++j)
      {
        jassert(state->getSubInference(j) == targetSubInference);
        Variable subSupervision = state->getSubSupervision(j);
        if (!filterUnsupervisedExamples || subSupervision.exists())
        {
          if (isValidationExample)
            res->addValidationExample(state->getSubInput(j), subSupervision);
          else
            res->addTrainingExample(state->getSubInput(j), subSupervision);
        }
      }
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_SHARED_PARALLEL_H_
