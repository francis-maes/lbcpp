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
  GradientDescentOnlineLearner(LearnerUpdateFrequency learningUpdateFrequency,
                                IterationFunctionPtr learningRate, bool normalizeLearningRate, 
                                LearnerUpdateFrequency regularizerUpdateFrequency, ScalarObjectFunctionPtr regularizer);
  GradientDescentOnlineLearner() : epoch(0), learningUpdateFrequency(never), normalizeLearningRate(false), regularizerUpdateFrequency(never), lastEmpiricalRisk(0.0) {}


  virtual void startLearningCallback(InferenceContext& context);
  virtual void stepFinishedCallback(InferenceContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(InferenceContext& context, const InferencePtr& inference);
  virtual void passFinishedCallback(InferenceContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput);

  virtual void getScores(std::vector< std::pair<String, double> >& res) const
    {res.push_back(std::make_pair(T("empiricalRisk"), lastEmpiricalRisk));}

  virtual double getDefaultScore() const
    {return -lastEmpiricalRisk;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class GradientDescentOnlineLearnerClass;

  ScalarVariableRecentMean numberOfActiveFeatures;
  size_t epoch;

  LearnerUpdateFrequency learningUpdateFrequency;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;

  LearnerUpdateFrequency regularizerUpdateFrequency;
  ScalarObjectFunctionPtr regularizer;
  CriticalSection lossValueLock;
  ScalarVariableMean lossValue;
  double lastEmpiricalRisk;
  size_t lastApplyRegularizerEpoch;

  void updateParameters(InferenceContext& context, const InferencePtr& inference, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, ObjectPtr* target = NULL);

  bool shouldApplyRegularizerAfterStep(size_t epoch) const;
  void applyRegularizer(ExecutionContext& context, const InferencePtr& inference);
  void checkRegularizerAfterStep(ExecutionContext& context, const InferencePtr& inference);
  void gradientDescentStep(ExecutionContext& context, const InferencePtr& inference, const ObjectPtr& gradient, double weight = 1.0);
  double computeLearningRate() const;
  void updateNumberOfActiveFeatures(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input);

private:
  NumericalInferencePtr getNumericalInference(const InferencePtr& inference) const;
  const PerceptionPtr& getPerception(const InferencePtr& inference) const
    {return getNumericalInference(inference)->getPerception();}
};

typedef ReferenceCountedObjectPtr<GradientDescentOnlineLearner> GradientDescentOnlineLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_ONLINE_LEARNER_GRADIENT_DESCENT_H_
