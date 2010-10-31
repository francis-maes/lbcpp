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
# include "../Perception/PerceptionRewriter.h"
# include "../Inference/InferenceOnlineLearner.h"

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
// Modifiers
extern InferencePtr addBiasInference(const String& name, double initialBias = 0.0);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

// Regression
extern StaticDecoratorInferencePtr squareRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr absoluteRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr dihedralAngleRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// Binary Classification
extern StaticDecoratorInferencePtr binaryLinearSVMInference(InferencePtr scoreInference);
extern StaticDecoratorInferencePtr binaryLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr binaryLogisticRegressionInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// MultiClass Classification
extern StaticDecoratorInferencePtr multiClassLinearSVMInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, bool updateOnlyMostViolatedClasses = false, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr multiClassMaxentInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// Ranking
extern StaticDecoratorInferencePtr bestAgainstAllRankingInference(InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss, InferenceOnlineLearnerPtr onlineLearner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr mostViolatedPairRankingInference(InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss, InferenceOnlineLearnerPtr onlineLearner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr allPairsRankingInference(InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss, InferenceOnlineLearnerPtr onlineLearner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr allPairsRankingLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern StaticDecoratorInferencePtr binaryClassificationRankingLinearSVMInference(PerceptionPtr perception, InferenceOnlineLearnerPtr onlineLearner, const String& name = T("unnamed"), bool optimizeMcc = false); // F1 by default

/*
** Batch Learner
*/
extern SequentialInferencePtr stochasticNumericalInferenceLearner(bool precomputePerceptions = true, bool randomizeExamples = true);
extern AtomicInferenceLearnerPtr addBiasInferenceLearner();

/*
** OnlineLearner
*/
// Gradient Descent Learner
extern InferenceOnlineLearnerPtr gradientDescentOnlineLearner(
          // randomization
          InferenceOnlineLearner::UpdateFrequency randomizationFrequency = InferenceOnlineLearner::never,
          // learning steps
          InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency = InferenceOnlineLearner::perEpisode,
          IterationFunctionPtr learningRate = constantIterationFunction(1.0),
          bool normalizeLearningRate = true,
          // regularizer
          InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency = InferenceOnlineLearner::perEpisode,
          ScalarObjectFunctionPtr regularizer = ScalarObjectFunctionPtr());

extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, const std::vector<NumericalInferencePtr>& targetInferences);
extern InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, NumericalInferencePtr targetInference);

extern UpdatableOnlineLearnerPtr addBiasOnlineLearner(InferenceOnlineLearner::UpdateFrequency updateFrequency);

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_H_
