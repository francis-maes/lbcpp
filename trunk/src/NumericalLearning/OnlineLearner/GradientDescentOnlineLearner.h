/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOnlineLearner.h | Base class for gradient         |
| Author  : Francis Maes                   |  descent learners               |
| Started : 25/05/2010 18:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_H_
# define LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_H_

# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Function/ScalarObjectFunction.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class GradientDescentOnlineLearner : public InferenceOnlineLearner
{
public:
  GradientDescentOnlineLearner(UpdateFrequency learningUpdateFrequency,
                                IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                UpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer);
  GradientDescentOnlineLearner() : epoch(0), learningUpdateFrequency(never), normalizeLearningRate(false), regularizerUpdateFrequency(never), lastEmpiricalLossValue(0.0) {}


  virtual void startLearningCallback();

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(const InferencePtr& inference);
  virtual void passFinishedCallback(const InferencePtr& inference);
  virtual double getCurrentLossEstimate() const
    {return lastEmpiricalLossValue;}
  
  virtual void clone(const ObjectPtr& target) const;

protected:
  friend class GradientDescentOnlineLearnerClass;

  ScalarVariableRecentMean numberOfActiveFeatures;
  size_t epoch;

  UpdateFrequency learningUpdateFrequency;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;

  UpdateFrequency regularizerUpdateFrequency;
  ScalarObjectFunctionPtr regularizer;
  CriticalSection lossValueLock;
  ScalarVariableMean lossValue;
  double lastEmpiricalLossValue;
  size_t lastApplyRegularizerEpoch;

  void updateParameters(const InferencePtr& inference, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, ObjectPtr* target = NULL);

  bool shouldApplyRegularizerAfterStep(size_t epoch) const;
  void applyExample(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  void applyRegularizer(const InferencePtr& inference);
  void checkRegularizerAfterStep(const InferencePtr& inference);
  void gradientDescentStep(const InferencePtr& inference, const ObjectPtr& gradient, double weight = 1.0);
  double computeLearningRate() const;
  void updateNumberOfActiveFeatures(const PerceptionPtr& perception, const Variable& input);

private:
  NumericalInferencePtr getNumericalInference(const InferencePtr& inference) const;
  const PerceptionPtr& getPerception(const InferencePtr& inference) const
    {return getNumericalInference(inference)->getPerception();}
};

typedef ReferenceCountedObjectPtr<GradientDescentOnlineLearner> GradientDescentOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_H_
