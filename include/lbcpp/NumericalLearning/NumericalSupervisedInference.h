/*-----------------------------------------.---------------------------------.
| Filename: NumericalSupervisedInference.h | Numerical Supervised Learning   |
| Author  : Francis Maes                   |                                 |
| Started : 01/11/2010 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_SUPERVISED_INFERENCE_H_
# define LBCPP_NUMERICAL_LEARNING_SUPERVISED_INFERENCE_H_

# include "NumericalInference.h"
# include "../Inference/InferenceOnlineLearner.h"
# include "../Inference/DecoratorInference.h"

namespace lbcpp
{

class NumericalSupervisedInference : public StaticDecoratorInference
{
public:
  NumericalSupervisedInference(const String& name, InferencePtr subInference)
    : StaticDecoratorInference(name, subInference) {}
  NumericalSupervisedInference() {}

  void setStochasticLearner(const InferenceOnlineLearnerPtr& onlineLearner, bool precomputePerceptions = true, bool randomizeExamples = true);
};

typedef ReferenceCountedObjectPtr<NumericalSupervisedInference> NumericalSupervisedInferencePtr;

// Regression
extern NumericalSupervisedInferencePtr squareRegressionInference(const String& name, PerceptionPtr perception);
extern NumericalSupervisedInferencePtr absoluteRegressionInference(const String& name, PerceptionPtr perception);
extern NumericalSupervisedInferencePtr dihedralAngleRegressionInference(const String& name, PerceptionPtr perception);

// Binary Classification
extern NumericalSupervisedInferencePtr binaryLinearSVMInference(InferencePtr scoreInference);
extern NumericalSupervisedInferencePtr binaryLinearSVMInference(const String& name, PerceptionPtr perception);
extern NumericalSupervisedInferencePtr binaryLogisticRegressionInference(const String& name, PerceptionPtr perception);

// MultiClass Classification
extern NumericalSupervisedInferencePtr multiClassLinearSVMInference(const String& name, PerceptionPtr perception, EnumerationPtr classes, bool updateOnlyMostViolatedClasses = false);
extern NumericalSupervisedInferencePtr multiClassMaxentInference(const String& name, PerceptionPtr perception, EnumerationPtr classes);

// Ranking
extern NumericalSupervisedInferencePtr bestAgainstAllRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss);
extern NumericalSupervisedInferencePtr mostViolatedPairRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss);
extern NumericalSupervisedInferencePtr allPairsRankingInference(const String& name, InferencePtr scoreInference, BinaryClassificationLossFunctionPtr baseLoss);
extern NumericalSupervisedInferencePtr allPairsRankingLinearSVMInference(const String& name, PerceptionPtr perception);
extern NumericalSupervisedInferencePtr binaryClassificationRankingLinearSVMInference(const String& name, PerceptionPtr perception);

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_H_
