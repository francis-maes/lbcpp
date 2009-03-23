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
# include "IterationFunction.h"

namespace cralgo
{

class GradientBasedLearner : public Object
{
public:
  static GradientBasedLearnerPtr createStochasticDescent(IterationFunctionPtr learningRate = IterationFunctionPtr(), bool normalizeLearningRate = true);
  static GradientBasedLearnerPtr createBatch(VectorOptimizerPtr optimizer, OptimizerTerminationTestPtr termination);
  
public:
  GradientBasedLearner() : meanInputSize(0.0) {}
  
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}
    
  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}
    
  void setMeanInputSize(double meanInputSize)
    {this->meanInputSize = meanInputSize;}

  virtual void trainStochasticBegin() {}
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight) = 0;
  virtual void trainStochasticExample(ScalarVectorFunctionPtr exampleLoss)
    {trainStochasticExample(exampleLoss->computeGradient(parameters), 1.0);}    
  virtual void trainStochasticEnd() {}

  virtual void trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples) = 0;

protected:
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
  double meanInputSize;
};

}; /* namespace cralgo */

#endif // !CRALGO_GRADIENT_BASED_LEARNER_H_
