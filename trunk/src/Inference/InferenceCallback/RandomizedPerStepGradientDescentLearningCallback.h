/*-----------------------------------------.---------------------------------.
| Filename: RandomizedPerStepGradientDe...h| Randomized stochastic gradient  |
| Author  : Francis Maes                   |  descent                        |
| Started : 25/05/2010 18:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_RANDOMIZED_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_RANDOMIZED_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_

# include "GradientDescentLearningCallback.h"

namespace lbcpp
{

class RandomizedPerStepGradientDescentLearningCallback : public GradientDescentLearningCallback
{
public:
  RandomizedPerStepGradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency randomizationFrequency,
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer)
    : GradientDescentLearningCallback(inference, perStep, learningRate, normalizeLearningRate,
                                      randomizationFrequency, regularizerUpdateFrequency, regularizer) {}

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput)
    {examples.push_back(Example(input, supervision, predictedOutput));}
 
  virtual void episodeFinishedCallback()
  {
    if (randomizationFrequency == perEpisode)
      flushExamples();
    GradientDescentLearningCallback::episodeFinishedCallback();
  }

  virtual void passFinishedCallback()
  {
    if (regularizerUpdateFrequency == perPass)
      applyRegularizer();
  }

private:
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
    std::vector<size_t> order;
    RandomGenerator::getInstance().sampleOrder(examples.size(), order);
    for (size_t i = 0; i < order.size(); ++i)
    {
      Example& example = examples[order[i]];
      applyExample(example.input, example.supervision, example.predictedOutput);
    }
    examples.clear();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_RANDOMIZED_PER_STEP_GRADIENT_DESCENT_LEARNING_CALLBACK_H_
