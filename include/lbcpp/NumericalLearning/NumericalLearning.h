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
# include "../Perception/PerceptionRewriter.h"
# include "../Inference/Inference.h"
# include "../Inference/InferenceOnlineLearner.h"
# include "../Inference/ParameterizedInference.h"

namespace lbcpp
{

/*
** Maths
*/
// Const-unary operations
extern size_t l0norm(const ObjectPtr& object);
extern size_t l0norm(const PerceptionPtr& perception, const Variable& input);

extern double l1norm(const ObjectPtr& object);
extern double l1norm(const PerceptionPtr& perception, const Variable& input);

extern double sumOfSquares(const ObjectPtr& object);
extern double sumOfSquares(const PerceptionPtr& perception, const Variable& input);

inline double l2norm(const ObjectPtr& object)
  {return object ? sqrt(sumOfSquares(object)) : 0.0;}

inline double l2norm(const PerceptionPtr& perception, const Variable& input)
  {return sqrt(sumOfSquares(perception, input));}

// Unary operations
extern void multiplyByScalar(const ObjectPtr& object, double scalar);

// Binary operations
extern double dotProduct(const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input);
extern double dotProduct(const ObjectPtr& object1, const ObjectPtr& object2);

extern void addWeighted(ObjectPtr& target, const ObjectPtr& source, double weight);
extern void addWeighted(ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight);

/*
** Perceptions
*/
extern PerceptionPtr selectAndMakeConjunctionFeatures(PerceptionPtr decorated, const ConjunctionVector& selectedConjunctions = ConjunctionVector());

// product perceptions
extern PerceptionPtr conjunctionFeatures(PerceptionPtr perception1, PerceptionPtr perception2);

// boolean / enumeration features
extern PerceptionPtr booleanFeatures();
extern PerceptionPtr enumValueFeatures(EnumerationPtr enumeration);

// number features
extern PerceptionPtr hardDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures);
extern PerceptionPtr softDiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures, bool cyclicBehavior);
extern PerceptionPtr softDiscretizedLogNumberFeatures(TypePtr inputType, double minimumLogValue, double maximumLogValue, size_t numIntervals, bool doOutOfBoundsFeatures);

extern PerceptionPtr signedNumberFeatures(PerceptionPtr positiveNumberPerception);

extern PerceptionPtr defaultPositiveIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultPositiveDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
extern PerceptionPtr defaultProbabilityFeatures(size_t numIntervals = 5);

// perception rewriter
extern PerceptionRewriteRulePtr enumValueFeaturesPerceptionRewriteRule();
extern PerceptionPtr perceptionToFeatures(PerceptionPtr perception);

/*
** Inferences
*/
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
extern NumericalInferencePtr linearInference(const String& name, PerceptionPtr perception);
extern NumericalInferencePtr multiLinearInference(const String& name, PerceptionPtr perception, ClassPtr outputClass);
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

/*
** OnlineLearner
*/
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

extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, const std::vector<NumericalInferencePtr>& targetInferences);
extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, NumericalInferencePtr targetInference);

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_H_
