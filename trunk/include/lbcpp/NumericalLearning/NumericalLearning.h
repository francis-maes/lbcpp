/*-----------------------------------------.---------------------------------.
| Filename: NumericalLearning.h            | Numerical Learning Header       |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_H_
# define LBCPP_NUMERICAL_LEARNING_H_

# include "LossFunctions.h"
# include "NumericalInference.h"
# include "NumericalSupervisedInference.h"
# include "../Perception/PerceptionRewriter.h"
# include "../Inference/InferenceOnlineLearner.h"
# include "../Function/IterationFunction.h"

namespace lbcpp
{

/*
** Maths
*/
// Const-unary operations
extern size_t l0norm(ExecutionContext& context, const ObjectPtr& object);
extern size_t l0norm(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input);

extern double l1norm(ExecutionContext& context, const ObjectPtr& object);
extern double l1norm(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input);

extern double sumOfSquares(ExecutionContext& context, const ObjectPtr& object);
extern double sumOfSquares(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input);

inline double l2norm(ExecutionContext& context, const ObjectPtr& object)
  {return object ? sqrt(sumOfSquares(context, object)) : 0.0;}

inline double l2norm(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input)
  {return sqrt(sumOfSquares(context, perception, input));}

// Unary operations
extern void multiplyByScalar(ExecutionContext& context, const ObjectPtr& object, double scalar);

// Binary operations
extern double dotProduct(ExecutionContext& context, const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input);
extern double dotProduct(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2);

extern void addWeighted(ExecutionContext& context, ObjectPtr& target, const ObjectPtr& source, double weight);
extern void addWeighted(ExecutionContext& context, ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight);

/*
** Perceptions
*/
extern PerceptionPtr addUnitFeatures(TypePtr inputType);

extern PerceptionPtr selectAndMakeConjunctionFeatures(PerceptionPtr decorated, const ConjunctionVector& selectedConjunctions = ConjunctionVector());

// product perceptions
extern PerceptionPtr conjunctionFeatures(PerceptionPtr perception1, PerceptionPtr perception2, bool singleInputForBothPerceptions = true);

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
// Modifiers
extern InferencePtr addBiasInference(const String& name, double initialBias = 0.0);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

/*
** Batch Learner
*/
extern StaticDecoratorInferencePtr precomputePerceptionsNumericalInferenceLearner(InferencePtr baseLearner);
extern InferencePtr addBiasInferenceLearner();

/*
** OnlineLearner
*/
// Gradient Descent Learner
extern InferenceOnlineLearnerPtr gradientDescentOnlineLearner(
          // learning steps
          LearnerUpdateFrequency learningUpdateFrequency = perEpisode,
          IterationFunctionPtr learningRate = constantIterationFunction(1.0),
          bool normalizeLearningRate = true,
          // regularizer
          LearnerUpdateFrequency regularizerUpdateFrequency = perEpisode,
          ScalarObjectFunctionPtr regularizer = ScalarObjectFunctionPtr());

extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, const std::vector<NumericalInferencePtr>& targetInferences);
extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, NumericalInferencePtr targetInference);

extern UpdatableOnlineLearnerPtr addBiasOnlineLearner(LearnerUpdateFrequency updateFrequency);

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_H_
