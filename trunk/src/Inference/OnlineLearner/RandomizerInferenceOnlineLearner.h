/*-----------------------------------------.---------------------------------.
| Filename: RandomizerLearningInference...h| A learner that randomizes       |
| Author  : Francis Maes                   |  examples order                 |
| Started : 25/05/2010 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_

# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>

namespace lbcpp
{

class RandomizerInferenceOnlineLearner : public InferenceOnlineLearner
{
public:
  RandomizerInferenceOnlineLearner(UpdateFrequency randomizationFrequency, InferenceOnlineLearnerPtr targetLearningCallback)
    : randomizationFrequency(randomizationFrequency), targetLearningCallback(targetLearningCallback) {}

  RandomizerInferenceOnlineLearner()
    : randomizationFrequency(never) {}

  virtual void startLearningCallback()
    {targetLearningCallback->startLearningCallback();}

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    storeExample(input, supervision, prediction);
    if (randomizationFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = randomizationFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (examples.size() % miniBatchSize) == 0)
        flushExamples(inference);
    }
  }
 
  virtual void episodeFinishedCallback(const InferencePtr& inference)
  {
    if (randomizationFrequency == perEpisode || randomizationFrequency >= perStepMiniBatch + 1)
      flushExamples(inference);
    if (randomizationFrequency != perPass)
      targetLearningCallback->episodeFinishedCallback(inference);
  }

  virtual void passFinishedCallback(const InferencePtr& inference)
  {
    if (randomizationFrequency == perPass)
    {
      flushExamples(inference);
      targetLearningCallback->episodeFinishedCallback(inference);
    }
    targetLearningCallback->passFinishedCallback(inference);
  }
  
  virtual double getCurrentLossEstimate() const
    {return targetLearningCallback->getCurrentLossEstimate();}

  virtual ObjectPtr clone() const
    {return new RandomizerInferenceOnlineLearner(randomizationFrequency, targetLearningCallback->cloneAndCast<InferenceOnlineLearner>());}

private:
  UpdateFrequency randomizationFrequency;
  InferenceOnlineLearnerPtr targetLearningCallback;

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

  void flushExamples(InferencePtr inference)
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
      targetLearningCallback->stepFinishedCallback(inference, example.input, example.supervision, Variable()/*example.prediction*/);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
