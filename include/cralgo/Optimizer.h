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

class ProgressCallback
{
public:
  virtual ~ProgressCallback() {}
  
  virtual void progressStart(const std::string& description)
    {}
    
  // return false to stop the task
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {return true;}
    
  virtual void progressEnd()
    {}
};

class ConsoleProgressCallback : public ProgressCallback
{
public:
  virtual void progressBegin(const std::string& description)
    {std::cout << "Begin '" << description << "'" << std::endl;}
    
  // return false to stop the task
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
    {std::cout << "Step '" << description << "' iteration = " << iteration << " / " << totalIterations << std::endl; return true;}
    
  virtual void progressEnd()
    {std::cout << "End." << std::endl;}
};

class OptimizerTerminationTest : public Object
{
public:
  static OptimizerTerminationTestPtr createMaxIterations(size_t maxIterations);
  static OptimizerTerminationTestPtr createAverageImprovementThreshold(double tolerance);

public:
  virtual void reset() = 0;

  virtual bool isTerminated(double value, double parameter, double derivative) = 0;
  virtual bool isTerminated(double value, const DenseVectorPtr parameters, const DenseVectorPtr gradient) = 0;
};

class ScalarOptimizer : public Object
{
public:
  virtual bool optimize(ScalarFunctionPtr function, double& value, OptimizerTerminationTestPtr termination, ProgressCallback& callback) = 0;
};

class VectorOptimizer : public Object
{
public:
  static VectorOptimizerPtr createGradientDescent(IterationFunctionPtr stepSize);
  static VectorOptimizerPtr createRProp();

public:
  virtual bool optimize(ScalarVectorFunctionPtr function, DenseVectorPtr& parameters, OptimizerTerminationTestPtr termination, ProgressCallback& callback) = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_OPTIMIZER_H_

