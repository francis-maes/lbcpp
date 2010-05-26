/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.h            | Inference Callback base class   |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_H_
# define LBCPP_INFERENCE_CALLBACK_H_

# include "Inference.h"
# include "../ObjectPredeclarations.h"
# include "../Utilities/RandomVariable.h"
# include "../Utilities/IterationFunction.h"

namespace lbcpp
{

class InferenceCallback : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual void startInferencesCallback(size_t count)
    {}

  virtual void finishInferencesCallback()
    {}

  // this function may modify the input or the supervision
  // it may also set an output, which causes the current inference step to be skipped
  // the function may also set a returnCode != Inference::finishedReturnCode to skip the inference step
  virtual void preInferenceCallback(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
    {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}

  virtual void regressionCallback(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
    {}
};

extern InferenceCallbackPtr cacheInferenceCallback(InferenceResultCachePtr cache, InferencePtr parentStep);
extern InferenceCallbackPtr cancelAfterStepCallback(InferencePtr lastStepBeforeBreak);

class LearningInferenceCallback : public InferenceCallback
{
public:
  LearningInferenceCallback(InferencePtr inference)
    : inference(inference) {}

  enum UpdateFrequency
  {
    never,
    perStep,
    perEpisode,
    perPass,
    perStepMiniBatch,
    perStepMiniBatch2 = perStepMiniBatch + 2,
    perStepMiniBatch5 = perStepMiniBatch + 5,
    perStepMiniBatch10 = perStepMiniBatch + 10,
    perStepMiniBatch20 = perStepMiniBatch + 20,
    perStepMiniBatch50 = perStepMiniBatch + 50,
    perStepMiniBatch100 = perStepMiniBatch + 100,
    perStepMiniBatch200 = perStepMiniBatch + 200,
    perStepMiniBatch500 = perStepMiniBatch + 500,
    perStepMiniBatch1000 = perStepMiniBatch + 1000,
  };

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput) = 0;
  virtual void episodeFinishedCallback() = 0;
  virtual void passFinishedCallback() = 0;

protected:
  InferencePtr inference;

  virtual void finishInferencesCallback();
  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode);
};

typedef ReferenceCountedObjectPtr<LearningInferenceCallback> LearningInferenceCallbackPtr;

extern LearningInferenceCallbackPtr stochasticDescentLearningCallback(LearnableAtomicInferencePtr inference, 
                                          LearningInferenceCallback::UpdateFrequency randomizationFrequency = LearningInferenceCallback::never,
                                          LearningInferenceCallback::UpdateFrequency learningUpdateFrequency = LearningInferenceCallback::perEpisode,
                                          IterationFunctionPtr learningRate = constantIterationFunction(1.0),
                                          bool normalizeLearningRate = true,
                                          LearningInferenceCallback::UpdateFrequency regularizerUpdateFrequency = LearningInferenceCallback::perEpisode,
                                          ScalarVectorFunctionPtr regularizer = ScalarVectorFunctionPtr());

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CALLBACK_H_
