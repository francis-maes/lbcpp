/*-----------------------------------------.---------------------------------.
| Filename: NumericalLearning.h            | Numerical Learning Header       |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_H_
# define LBCPP_NUMERICAL_LEARNING_H_

# include "../Perception/Perception.h"
# include "../Inference/Inference.h"
# include "../Inference/InferenceOnlineLearner.h"
# include "../Inference/ParameterizedInference.h"

namespace lbcpp
{

class NumericalInference : public ParameterizedInference
{
public:
  NumericalInference(const String& name, PerceptionPtr perception);
  NumericalInference() {}

  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  TypePtr getPerceptionOutputType() const
    {return perception->getOutputType();}

  virtual Variable predict(const Variable& input) const = 0;

  // if target == NULL, target is this parameters
  // supervision is the loss function
  //   ScalarFunction for single output machines
  //   ObjectScalarFunction for multiple output machines
  // parameters += weight * gradient(input, supervision=lossFunction, prediction)
  // exampleLossValue = loss(prediction) (supervision=lossFunction)
  virtual void computeAndAddGradient(double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target) = 0;

  void addWeightedToParameters(const ObjectPtr& value, double weight);
  void applyRegularizerToParameters(ScalarObjectFunctionPtr regularizer, double weight);

  const PerceptionPtr& getPerception() const
    {return perception;}

protected:
  friend class NumericalInferenceClass;

  PerceptionPtr perception;

  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return predict(input);}
};

typedef ReferenceCountedObjectPtr<NumericalInference> NumericalInferencePtr;

// Atomic
extern InferencePtr linearInference(const String& name, PerceptionPtr perception);
extern InferencePtr multiLinearInference(const String& name, PerceptionPtr perception, ClassPtr outputClass);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

// Binary Classification
extern InferencePtr binaryLinearSVMInference(InferencePtr scoreInference);
extern InferencePtr binaryLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr binaryLogisticRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// Regression
extern InferencePtr squareRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr absoluteRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr dihedralAngleRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// MultiClass Classification
extern StaticDecoratorInferencePtr multiClassLinearSVMInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, bool updateOnlyMostViolatedClasses = false, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr multiClassMaxentInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// Gradient Descent Learner
extern InferenceOnlineLearnerPtr gradientDescentInferenceOnlineLearner(
          // randomization
          InferenceOnlineLearner::UpdateFrequency randomizationFrequency = InferenceOnlineLearner::never,
          // learning steps
          InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency = InferenceOnlineLearner::perEpisode,
          IterationFunctionPtr learningRate = constantIterationFunction(1.0),
          bool normalizeLearningRate = true,
          // regularizer
          InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency = InferenceOnlineLearner::perEpisode,
          ScalarObjectFunctionPtr regularizer = ScalarObjectFunctionPtr(),
          // stopping criterion
          InferenceOnlineLearner::UpdateFrequency criterionTestFrequency = InferenceOnlineLearner::never,
          StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(),
          bool restoreBestParametersWhenLearningStops = false
    );

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_H_
