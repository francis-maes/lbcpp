/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.h         | Gradient-based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_H_
# define LBCPP_GRADIENT_BASED_LEARNER_H_

# include "ContinuousFunction.h"
# include "IterationFunction.h"

namespace lbcpp
{

class GradientBasedLearner : public Object
{
public:
  static GradientBasedLearnerPtr createStochasticDescent(IterationFunctionPtr learningRate = IterationFunctionPtr(), bool normalizeLearningRate = true);
  static GradientBasedLearnerPtr createBatch(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr termination);
  static GradientBasedLearnerPtr createBatch(VectorOptimizerPtr optimizer, size_t maxIterations = 100, double tolerance = 0.0001);
  static GradientBasedLearnerPtr createNonLearner();
  
public:
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}
    
  DenseVectorPtr getParameters() const
    {return parameters;}
    
  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}
    
  virtual void trainStochasticBegin() {}
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight) = 0;
  virtual void trainStochasticExample(ScalarVectorFunctionPtr exampleLoss)
    {trainStochasticExample(exampleLoss->computeGradient(parameters), 1.0);}    
  virtual void trainStochasticEnd() {}

  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallback* progress)
  {
    error("GradientBasedLearner::trainBatch", "This is a non-batch learner");
    return false;
  }

protected:
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_H_
