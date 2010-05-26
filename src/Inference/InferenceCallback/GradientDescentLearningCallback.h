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
  GradientDescentLearningCallback(ParameterizedInferencePtr inference,
                                  UpdateFrequency learningUpdateFrequency,
                                  IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                  UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer);

  virtual void stepFinishedCallback(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  virtual void episodeFinishedCallback();
  virtual void passFinishedCallback();
  virtual double getCurrentLossEstimate() const
    {return lossValue.getMean();}
  
protected:
  UpdateFrequency learningUpdateFrequency;
  UpdateFrequency regularizerUpdateFrequency;
  ScalarVectorFunctionPtr regularizer;
  ScalarVariableMean lossValue;
  size_t lastApplyRegularizerEpoch;

  FeatureGeneratorPtr getExampleGradient(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  bool shouldApplyRegularizerAfterStep(size_t epoch) const;
  void applyExample(ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  void applyRegularizer();
  void checkRegularizerAfterStep();
  void gradientDescentStep(FeatureGeneratorPtr gradient, double weight = 1.0);
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_
