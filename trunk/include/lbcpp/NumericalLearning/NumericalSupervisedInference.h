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
# include "LossFunctions.h"

namespace lbcpp
{

class NumericalSupervisedInference : public StaticDecoratorInference
{
public:
  NumericalSupervisedInference(const String& name, InferencePtr subInference)
    : StaticDecoratorInference(name, subInference) {}
  NumericalSupervisedInference() {}

  void setStochasticLearner(const InferenceOnlineLearnerPtr& onlineLearner,
                            bool precomputePerceptions = true, // this makes learning much faster, but may consume lots of memory
                            bool randomizeExamples = true); // randomize examples at each learning pass
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
