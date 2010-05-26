/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentLearningCallback.h| Base class for gradient       |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_

# include <lbcpp/Inference/InferenceCallback.h>
# include <lbcpp/Inference/InferenceBaseClasses.h>

namespace lbcpp
{

class IterativeLearningInferenceCallback : public LearningInferenceCallback
{
public:
  IterativeLearningInferenceCallback(InferencePtr inference, IterationFunctionPtr learningRate, bool normalizeLearningRate);

protected:
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  ScalarVariableMean numberOfActiveFeatures;
  size_t epoch;
 
  double computeLearningRate() const;
  void updateNumberOfActiveFeatures(FeatureGeneratorPtr features);
};

class GradientDescentLearningCallback : public IterativeLearningInferenceCallback
{
public:
  GradientDescentLearningCallback(LearnableAtomicInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency randomizationFrequency,
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer);

  virtual void episodeFinishedCallback();
  virtual void passFinishedCallback();
  
  LearnableAtomicInferencePtr getInference() const
    {return inference.staticCast<LearnableAtomicInference>();}

  DenseVectorPtr getParameters() const
    {return getInference()->getParameters();}

protected:
  UpdateFrequency learningUpdateFrequency;
  UpdateFrequency randomizationFrequency;
  UpdateFrequency regularizerUpdateFrequency;
  ScalarVectorFunctionPtr regularizer;
  ScalarVariableMean lossValue;

  FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  bool shouldApplyRegularizerAfterStep(size_t epoch) const;
  void applyExample(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  void applyRegularizer();
  void checkRegularizerAfterStep();
  void parametersChanged();

  virtual void finishInferencesCallback();
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_
