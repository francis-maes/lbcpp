/*-----------------------------------------.---------------------------------.
| Filename: RandomizerLearningInference...h| A learner that randomizes       |
| Author  : Francis Maes                   |  examples order                 |
| Started : 25/05/2010 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_

# include "GradientDescentLearningCallback.h"

namespace lbcpp
{

class RandomizerLearningInferenceCallback : public LearningInferenceCallback
{
public:
  RandomizerLearningInferenceCallback(InferencePtr inference, UpdateFrequency randomizationFrequency, LearningInferenceCallbackPtr targetLearningCallback)
    : LearningInferenceCallback(inference), randomizationFrequency(randomizationFrequency), targetLearningCallback(targetLearningCallback) {}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
  {
    examples.push_back(Example(input, supervision, predictedOutput));
    if (randomizationFrequency >= perStepMiniBatch)
    {
      int miniBatchSize = randomizationFrequency - perStepMiniBatch;
      if (miniBatchSize <= 1 || (examples.size() % miniBatchSize) == 0)
        flushExamples();
    }
  }
 
  virtual void episodeFinishedCallback()
  {
    if (randomizationFrequency == perEpisode || randomizationFrequency >= perStepMiniBatch + 1)
      flushExamples();
    targetLearningCallback->episodeFinishedCallback();
  }

  virtual void passFinishedCallback()
  {
    if (randomizationFrequency == perPass)
      flushExamples();
    targetLearningCallback->passFinishedCallback();
  }

private:
  UpdateFrequency randomizationFrequency;
  LearningInferenceCallbackPtr targetLearningCallback;

  struct Example
  {
    Example(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
      : input(input), supervision(supervision), predictedOutput(predictedOutput) {}

    ObjectPtr input;
    ObjectPtr supervision;
    ObjectPtr predictedOutput;
  };

  std::vector<Example> examples;
 
  void flushExamples()
  {
    if (!examples.size())
      return;
    //std::cout << "*" << std::flush;
    std::vector<size_t> order;
    RandomGenerator::getInstance().sampleOrder(examples.size(), order);
    for (size_t i = 0; i < order.size(); ++i)
    {
      Example& example = examples[order[i]];
      targetLearningCallback->stepFinishedCallback(example.input, example.supervision, example.predictedOutput);
    }
    examples.clear();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_RANDOMIZER_LEARNING_H_
