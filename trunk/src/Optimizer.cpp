/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.cpp                  | Optimizers code                 |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/Optimizer.h>
#include <cralgo/impl/Optimizer/OptimizerStaticToDynamic.hpp>
#include <cralgo/impl/Optimizer/RPropOptimizer.hpp>
#include <cralgo/impl/Optimizer/GradientDescentOptimizer.hpp>
#include <deque>
using namespace cralgo;

class MaxIterationsOptimizerTerminationTest : public OptimizerTerminationTest
{
public:
  MaxIterationsOptimizerTerminationTest(size_t maxIterations)
    : maxIterations(maxIterations) {}

  virtual void reset()
    {iterations = 0;}

  virtual bool isTerminated(double, double, double)
    {++iterations; return iterations >= maxIterations;}
    
  virtual bool isTerminated(double, const DenseVectorPtr, const DenseVectorPtr)
    {++iterations; return iterations >= maxIterations;}

private:
  size_t iterations, maxIterations;
};

OptimizerTerminationTestPtr OptimizerTerminationTest::createMaxIterations(size_t maxIterations)
  {return new MaxIterationsOptimizerTerminationTest(maxIterations);}

class AverageImprovementOptimizerTerminationTest : public OptimizerTerminationTest
{
public:
  AverageImprovementOptimizerTerminationTest(double tolerance = 0.001)
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
    
  virtual bool isTerminated(double value, const DenseVectorPtr, const DenseVectorPtr)
    {return isTerminated(value);}

private:
  double tolerance;
  std::deque<double> prevs;
};

OptimizerTerminationTestPtr OptimizerTerminationTest::createAverageImprovementThreshold(double tolerance)
  {return new AverageImprovementOptimizerTerminationTest(tolerance);}

VectorOptimizerPtr VectorOptimizer::createRProp()
  {return impl::staticToDynamic(impl::RPropOptimizer());}

VectorOptimizerPtr VectorOptimizer::createGradientDescent(IterationFunctionPtr stepSize)
  {return impl::staticToDynamic(impl::GradientDescentOptimizer(stepSize));}
