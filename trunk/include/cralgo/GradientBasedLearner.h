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

class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;

class IterationFunction : public Object
{
public:
  static IterationFunctionPtr createConstant(double value);

  virtual double compute(size_t iteration) const = 0;
};

class GradientBasedLearner;
typedef ReferenceCountedObjectPtr<GradientBasedLearner> GradientBasedLearnerPtr;

class GradientBasedLearner : public Object
{
public:
  virtual void trainStochasticBegin(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer) {}
  virtual void trainStochasticExample(DenseVectorPtr parameters, ScalarVectorFunctionPtr exampleLoss, ScalarVectorFunctionPtr regularizer) = 0;
  virtual void trainStochasticEnd(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer) {}
  virtual void trainBatch(DenseVectorPtr parameters, ScalarVectorFunctionPtr objective, size_t numExamples) = 0;

  static GradientBasedLearnerPtr createGradientDescent(IterationFunctionPtr learningRate = IterationFunctionPtr(), bool normalizeLearningRate = true);
};

}; /* namespace cralgo */

#endif // !CRALGO_GRADIENT_BASED_LEARNER_H_
