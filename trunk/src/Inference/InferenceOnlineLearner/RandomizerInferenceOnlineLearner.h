/*-----------------------------------------.---------------------------------.
| Filename: RandomizerLearningInference...h| A learner that randomizes       |
| Author  : Francis Maes                   |  examples order                 |
| Started : 25/05/2010 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_

# include "GradientDescentOnlineLearner.h"

namespace lbcpp
{

class RandomizerInferenceOnlineLearner : public InferenceOnlineLearner
{
public:
  RandomizerInferenceOnlineLearner(UpdateFrequency randomizationFrequency, InferenceOnlineLearnerPtr targetLearningCallback)
    : randomizationFrequency(randomizationFrequency), targetLearningCallback(targetLearningCallback) {}

  RandomizerInferenceOnlineLearner()
    : randomizationFrequency(never) {}

  virtual void stepFinishedCallback(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    examples.push_back(Example(input, supervision, prediction));
    if (randomizationFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = randomizationFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (examples.size() % miniBatchSize) == 0)
        flushExamples(inference);
    }
  }
 
  virtual void episodeFinishedCallback(InferencePtr inference)
  {
    if (randomizationFrequency == perEpisode || randomizationFrequency >= perStepMiniBatch + 1)
      flushExamples(inference);
    targetLearningCallback->episodeFinishedCallback(inference);
  }

  virtual void passFinishedCallback(InferencePtr inference)
  {
    if (randomizationFrequency == perPass)
      flushExamples(inference);
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
      : input(input), supervision(supervision), prediction(prediction) {}

    Variable input;
    Variable supervision;
    Variable prediction;
  };

  std::vector<Example> examples;
 
  void flushExamples(InferencePtr inference)
  {
    if (!examples.size())
      return;
    //std::cout << "*" << std::flush;
    std::vector<size_t> order;
    RandomGenerator::getInstance().sampleOrder(examples.size(), order);
    for (size_t i = 0; i < order.size(); ++i)
    {
      const Example& example = examples[order[i]];
      targetLearningCallback->stepFinishedCallback(inference, example.input, example.supervision, example.prediction);
    }
    examples.clear();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
