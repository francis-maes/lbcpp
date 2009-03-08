/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.h         | Gradient-based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_GRADIENT_BASED_LEARNER_H_
# define CRALGO_GRADIENT_BASED_LEARNER_H_

# include "ContinuousFunction.h"

namespace cralgo
{

class GradientBasedLearner : public Object
{
public:
  virtual void trainStochasticBegin(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer) {}
  virtual void trainStochasticExample(DenseVectorPtr parameters, const FeatureGeneratorPtr input, ScalarVectorFunctionPtr loss, ScalarVectorFunctionPtr regularizer) = 0;
  virtual void trainStochasticEnd(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer) {}
  virtual void trainBatch(DenseVectorPtr parameters, ScalarVectorFunctionPtr objective) = 0;
};

typedef ReferenceCountedObjectPtr<GradientBasedLearner> GradientBasedLearnerPtr;

extern GradientBasedLearnerPtr createGradientDescentLearner();

}; /* namespace cralgo */

#endif // !CRALGO_GRADIENT_BASED_LEARNER_H_
