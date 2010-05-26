/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOnlineLearner.h | Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_
# define LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_

# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/InferenceBaseClasses.h>

namespace lbcpp
{

class GradientDescentOnlineLearner : public InferenceOnlineLearner
{
public:
  GradientDescentOnlineLearner(UpdateFrequency learningUpdateFrequency,
                                IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer);
  GradientDescentOnlineLearner() : epoch(0), learningUpdateFrequency(never), normalizeLearningRate(false), regularizerUpdateFrequency(never) {}

  virtual void stepFinishedCallback(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  virtual void episodeFinishedCallback(InferencePtr inference);
  virtual void passFinishedCallback(InferencePtr inference);
  virtual double getCurrentLossEstimate() const
    {return lossValue.getMean();}
  
  virtual ObjectPtr clone() const;

protected:
  ScalarVariableMean numberOfActiveFeatures;
  size_t epoch;

  UpdateFrequency learningUpdateFrequency;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;

  UpdateFrequency regularizerUpdateFrequency;
  ScalarVectorFunctionPtr regularizer;
  ScalarVariableMean lossValue;
  size_t lastApplyRegularizerEpoch;

  FeatureGeneratorPtr getExampleGradient(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  bool shouldApplyRegularizerAfterStep(size_t epoch) const;
  void applyExample(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ObjectPtr predictedOutput);
  void applyRegularizer(InferencePtr inference);
  void checkRegularizerAfterStep(InferencePtr inference);
  void gradientDescentStep(InferencePtr inference, FeatureGeneratorPtr gradient, double weight = 1.0);
  double computeLearningRate() const;
  void updateNumberOfActiveFeatures(FeatureGeneratorPtr features);
};

typedef ReferenceCountedObjectPtr<GradientDescentOnlineLearner> GradientDescentOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CALLBACK_GRADIENT_DESCENT_LEARNING_H_
