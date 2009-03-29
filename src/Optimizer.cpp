/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Optimizers code                 |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/Optimizer.h>
#include "Optimizer/RPropOptimizer.h"
#include "Optimizer/GradientDescentOptimizer.h"
#include <deque>
using namespace cralgo;

class MaxIterationsOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  MaxIterationsOptimizerStoppingCriterion(size_t maxIterations)
    : maxIterations(maxIterations) {}

  virtual void reset()
    {iterations = 0;}

  virtual bool isTerminated(double, double, double)
    {++iterations; return iterations >= maxIterations;}
    
  virtual bool isTerminated(double, const FeatureGeneratorPtr, const FeatureGeneratorPtr)
    {++iterations; return iterations >= maxIterations;}

private:
  size_t iterations, maxIterations;
};

OptimizerStoppingCriterionPtr OptimizerStoppingCriterion::createMaxIterations(size_t maxIterations)
  {return new MaxIterationsOptimizerStoppingCriterion(maxIterations);}

class AverageImprovementOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  AverageImprovementOptimizerStoppingCriterion(double tolerance = 0.001)
    : tolerance(tolerance) {}
    
  virtual void reset()
    {prevs.clear();}

  bool isTerminated(double value)
  {
    if (prevs.size())
    {
      double prevVal = prevs.front();
      /*if (energy > prevVal)
        prevs.clear();
      else */if (prevs.size() >= 5)
      {
        double averageImprovement = (prevVal - value) / prevs.size();
        double relAvgImpr = value ? averageImprovement / fabs(value) : 0;
        //std::cout << "Av-Improvment: " << averageImprovement << " RealImprovment: " << relAvgImpr << " tol = " << getTolerance() << std::endl;
        if (relAvgImpr >= 0 && relAvgImpr <= tolerance)
          return true;
        prevs.pop_front();
      }
    }
    prevs.push_back(value);
    return false;
  }

  virtual bool isTerminated(double value, double, double)
    {return isTerminated(value);}
    
  virtual bool isTerminated(double value, const FeatureGeneratorPtr, const FeatureGeneratorPtr)
    {return isTerminated(value);}

private:
  double tolerance;
  std::deque<double> prevs;
};

OptimizerStoppingCriterionPtr OptimizerStoppingCriterion::createAverageImprovementThreshold(double tolerance)
  {return new AverageImprovementOptimizerStoppingCriterion(tolerance);}

class OrOptimizerStoppingCriterion : public OptimizerStoppingCriterion
{
public:
  OrOptimizerStoppingCriterion(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2)
    : criterion1(criterion1), criterion2(criterion2) {}

  virtual void reset()
    {criterion1->reset(); criterion2->reset();}

  virtual bool isTerminated(double value, double parameter, double derivative)
  {
    bool t1 = criterion1->isTerminated(value, parameter, derivative);
    bool t2 = criterion2->isTerminated(value, parameter, derivative);
    return t1 || t2;
  }
  
  virtual bool isTerminated(double value, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr gradient)
  {
    bool t1 = criterion1->isTerminated(value, parameters, gradient);
    bool t2 = criterion2->isTerminated(value, parameters, gradient);
    return t1 || t2;
  }

private:
  OptimizerStoppingCriterionPtr criterion1;
  OptimizerStoppingCriterionPtr criterion2;
};

OptimizerStoppingCriterionPtr OptimizerStoppingCriterion::createOr(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2)
  {return new OrOptimizerStoppingCriterion(criterion1, criterion2);}

/*
** Vector Optimizer
*/
VectorOptimizerPtr VectorOptimizer::createRProp()
  {return new RPropOptimizer();}

VectorOptimizerPtr VectorOptimizer::createGradientDescent(IterationFunctionPtr stepSize)
  {return new GradientDescentOptimizer(stepSize);}

bool VectorOptimizer::optimize(ScalarVectorFunctionPtr function, DenseVectorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress)
{
  FeatureGeneratorPtr p = parameters;
  bool res = optimize(function, p, stoppingCriterion, progress);
  parameters = p->toDenseVector();
  return res;
}

bool VectorOptimizer::optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallback* progress)
{
  if (!initialize(function, parameters))
    return false;
  
  for (size_t i = 0; true; ++i)
  {
    double value;
    FeatureGeneratorPtr gradient;
    function->compute(parameters, &value, FeatureGeneratorPtr(), &gradient);
    if (progress && !progress->progressStep("Optimizing, f = " + cralgo::toString(value) + " norm = " + cralgo::toString(parameters->l2norm()), (double)i))
      return false;
    if (stoppingCriterion->isTerminated(value, parameters, gradient))
      break;
    OptimizerState state = step(function, parameters, value, gradient);
    if (state == optimizerError)
      return false;
    else if (state == optimizerDone)
      return true;
    if (!parameters)
    {
      error("VectorOptimizer::optimize", "null parameters");
      return false;
    }
  }
  return true;
}
