/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Continuous Function Optimizers  |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_OPTIMIZER_H_
# define CRALGO_OPTIMIZER_H_

# include "ContinuousFunction.h"

namespace cralgo
{

class OptimizerStoppingCriterion : public Object
{
public:
  static OptimizerStoppingCriterionPtr createMaxIterations(size_t maxIterations);
  static OptimizerStoppingCriterionPtr createAverageImprovementThreshold(double tolerance);
  static OptimizerStoppingCriterionPtr createOr(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2);
  
public:
  virtual void reset() = 0;

  virtual bool isTerminated(double value, double parameter, double derivative) = 0;
  virtual bool isTerminated(double value, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr gradient) = 0;
};

enum OptimizerState
{
  optimizerError,
  optimizerContinue,
  optimizerDone,
};

class ScalarOptimizer : public Object
{
public:
  virtual bool optimize(ScalarFunctionPtr function, double& value, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress = NULL);

protected:
  virtual bool initialize(ScalarFunctionPtr function, double parameter) = 0;
  virtual OptimizerState step(ScalarFunctionPtr function, double& parameter, double value, double derivative) = 0;
};

class VectorOptimizer : public Object
{
public:
  static VectorOptimizerPtr createGradientDescent(IterationFunctionPtr stepSize);
  static VectorOptimizerPtr createRProp();

public:
  virtual bool optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress = NULL);
  virtual bool optimize(ScalarVectorFunctionPtr function, DenseVectorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress = NULL);

protected:
  virtual bool initialize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr parameters) = 0;
  virtual OptimizerState step(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, double value, FeatureGeneratorPtr gradient) = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_H_

