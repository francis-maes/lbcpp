/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Continuous Function Optimizers  |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "ContinuousFunction.h"

namespace lbcpp
{

class OptimizerStoppingCriterion : public Object
{
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

extern OptimizerStoppingCriterionPtr maxIterationsStoppingCriterion(size_t maxIterations);
extern OptimizerStoppingCriterionPtr averageImprovementThresholdStoppingCriterion(double tolerance);
extern OptimizerStoppingCriterionPtr logicalOr(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2);

class ScalarOptimizer : public Object
{
public:
  virtual bool optimize(ScalarFunctionPtr function, double& value, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

protected:
  virtual bool initialize(ScalarFunctionPtr function, double parameter) = 0;
  virtual OptimizerState step(ScalarFunctionPtr function, double& parameter, double value, double derivative) = 0;
};

class VectorOptimizer : public Object
{
public:
  virtual bool optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

  FeatureGeneratorPtr optimize(ScalarVectorFunctionPtr function, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    FeatureGeneratorPtr res = new DenseVector();
    if (!optimize(function, res, stoppingCriterion, progress))
      return FeatureGeneratorPtr();
    return res;
  }

protected:
  size_t iteration;
  ScalarVectorFunctionPtr function;
  
  FeatureGeneratorPtr parameters;
  double value;
  FeatureGeneratorPtr gradientDirection;
  FeatureGeneratorPtr gradient;
  
  void setParameters(FeatureGeneratorPtr parameters)
  {
    this->parameters = parameters;
    gradient = FeatureGeneratorPtr();
    function->compute(parameters, &value, gradientDirection, &gradient);
    assert(gradient->getDictionary() == parameters->getDictionary());
  }
  
  void setParametersGradientAndValue(FeatureGeneratorPtr parameters, FeatureGeneratorPtr gradient, double value)
    {this->parameters = parameters; this->value = value; this->gradient = gradient;}

  virtual bool initialize()   {return true;}
  virtual OptimizerState step() = 0;
};

extern VectorOptimizerPtr gradientDescentOptimizer(IterationFunctionPtr stepSize);
extern VectorOptimizerPtr rpropOptimizer();
extern VectorOptimizerPtr lbfgsOptimizer();

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_

