/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/GradientBasedLearner.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

/*
** TODO: move somewhere
*/
class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value) : value(value) {}
  
  virtual double compute(size_t iteration) const
    {return value;}
    
private:
  double value;
};

IterationFunctionPtr IterationFunction::createConstant(double value)
  {return new ConstantIterationFunction(value);}

class InvLinearIterationFunction : public IterationFunction
{
public:
  InvLinearIterationFunction(double initialValue, size_t numberIterationsToReachHalfInitialValue)
    : initialValue(initialValue), numberIterationsToReachHalfInitialValue(numberIterationsToReachHalfInitialValue) {}
    
  virtual double compute(size_t iteration) const
    {return initialValue * numberIterationsToReachHalfInitialValue / (double)(numberIterationsToReachHalfInitialValue + iteration);}

private:
  double initialValue;
  size_t numberIterationsToReachHalfInitialValue;
};

IterationFunctionPtr IterationFunction::createInvLinear(double initialValue, size_t numberIterationsToReachHalfInitialValue)
  {return new InvLinearIterationFunction(initialValue, numberIterationsToReachHalfInitialValue);}

class GradientDescentLearner : public GradientBasedLearner
{
public:
  GradientDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
    : learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
    
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
//    std::cout << "GRADIENT ...." << std::endl << gradient->toString() << std::endl;
    parameters->addWeighted(gradient, -weight * computeAlpha(gradient));
    ++epoch;
  }
  
  virtual void trainStochasticEnd()
  {
    // apply regularizer
    if (regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());
  }

  virtual void trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples)
  {
    for (int i = 0; i < 100; ++i)
    {
      std::cout << "Iteration " << i << " objective = " << objective->compute(parameters) << std::endl; 
      LazyVectorPtr gradient = objective->computeGradient(parameters);
      parameters->addWeighted(gradient, -computeAlpha(gradient) * numExamples);
      epoch += numExamples;
    }
  }
  
protected:
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;
  
  double computeAlpha(LazyVectorPtr gradient = LazyVectorPtr())
  {
    //std::cout << "Alpha = 1.0";
    double res = 1.0;
    if (learningRate)
    {
      //std::cout << " x " << learningRate->compute(epoch);
      res *= learningRate->compute(epoch);
    }
    if (normalizeLearningRate && meanInputSize)
    {
      //std::cout << " / " << meanInputSize;
      res /= meanInputSize;
    }
    //std::cout << " = " << res << std::endl;
    return res;
  }
};

GradientBasedLearnerPtr GradientBasedLearner::createGradientDescent(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return GradientBasedLearnerPtr(new GradientDescentLearner(learningRate, normalizeLearningRate));}
