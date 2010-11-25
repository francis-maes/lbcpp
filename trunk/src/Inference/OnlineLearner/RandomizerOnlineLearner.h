/*-----------------------------------------.---------------------------------.
| Filename: RandomizerOnlineLearner.h      | A learner that randomizes       |
| Author  : Francis Maes                   |  examples order                 |
| Started : 25/05/2010 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONLINE_LEARNER_RANDOMIZER_H_
# define LBCPP_INFERENCE_ONLINE_LEARNER_RANDOMIZER_H_

# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class RandomizerOnlineLearner : public UpdatableOnlineLearner
{
public:
  RandomizerOnlineLearner(LearnerUpdateFrequency randomizationFrequency)
    : UpdatableOnlineLearner(randomizationFrequency) {}
  RandomizerOnlineLearner() {}
 
  virtual void subStepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    jassert(false); // not supported yet
  }

  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    storeExample(input, supervision, prediction);
    updateAfterStep(context, inference);
  }
 
  virtual void episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference)
  {
    if (updateFrequency >= perStepMiniBatch + 1)
      update(context, inference); // flush remaining examples
    if (updateFrequency != perPass)
      UpdatableOnlineLearner::episodeFinishedCallback(context, inference);
  }

  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
  {
    if (updateFrequency == perPass)
    {
      update(context, inference);
      nextLearner->episodeFinishedCallback(context, inference);
    }
    InferenceOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);
  }

  virtual void update(ExecutionContext& context, const InferencePtr& inference)
  {
    std::vector<Example> examples;
    getExamplesAndClear(examples);
    if (!examples.size())
      return;
    //std::cout << "*" << std::flush;
    std::vector<size_t> order;
    RandomGenerator::getInstance()->sampleOrder(examples.size(), order);
    for (size_t i = 0; i < order.size(); ++i)
    {
      const Example& example = examples[order[i]];
      nextLearner->stepFinishedCallback(context, inference, example.input, example.supervision, Variable()/*example.prediction*/);
    }
  }

private:
  struct Example
  {
    Example(const Variable& input, const Variable& supervision, const Variable& prediction)
      : input(input), supervision(supervision)/*, prediction(prediction)*/ {}

    Variable input;
    Variable supervision;
    //Variable prediction;
  };

  CriticalSection examplesLock;
  std::vector<Example> examples;
 
  void storeExample(const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    ScopedLock _(examplesLock);
    examples.push_back(Example(input, supervision, prediction));
  }

  void getExamplesAndClear(std::vector<Example>& examples)
  {
    ScopedLock _(examplesLock);
    this->examples.swap(examples);
    this->examples.clear();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONLINE_LEARNER_RANDOMIZER_H_
