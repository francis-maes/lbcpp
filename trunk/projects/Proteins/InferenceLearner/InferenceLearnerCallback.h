/*-----------------------------------------.---------------------------------.
| Filename: InferenceLearnerCallback.h     | Inference Learner Callback      |
| Author  : Francis Maes                   |                                 |
| Started : 16/04/2010 18:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LEARNER_CALLBACK_H_
# define LBCPP_INFERENCE_LEARNER_CALLBACK_H_

# include "../InferencePredeclarations.h"

namespace lbcpp
{

class InferenceLearnerCallback : public Object
{
public:
  virtual InferenceContextPtr createContext() = 0;

  virtual RegressorPtr createRegressor(InferenceStackPtr stack) = 0;
  virtual ClassifierPtr createClassifier(InferenceStackPtr stack, FeatureDictionaryPtr labels) = 0;

  virtual double getProbabilityToCreateAnExample(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision) = 0;
  virtual InferenceCallbackPtr getLearningCallback(LearnableAtomicInferenceStepPtr step, InferenceStepPtr parentStep)
    {return InferenceCallbackPtr();}

  //{std::cout << "Using default probability" << std::endl; return 1.0;}

  virtual void preLearningIterationCallback(size_t iterationNumber)
    {}

  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
    {return iterationNumber < 100;}

  virtual void preLearningStepCallback(InferenceStepPtr step)
    {}

  virtual void postLearningStepCallback(InferenceStepPtr step)
    {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LEARNER_CALLBACK_H_
