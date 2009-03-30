/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/GradientBasedLearner.h>
#include <cralgo/Optimizer.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

class GradientDescentLearner : public GradientBasedLearner
{
public:
  GradientDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
    : learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
    
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    assert(parameters);
//    std::cout << "GRADIENT ...." << std::endl << gradient->toString() << std::endl;
    //std::cout << "Params.addWeighted(" << gradient->toString() << " , " << (-weight * computeAlpha()) << ")" << std::endl;
    parameters->addWeighted(gradient, -weight * computeAlpha());
    //std::cout << " => " << parameters->toString() << std::endl;
    ++epoch;
  }
  
  virtual void trainStochasticEnd()
  {
    // apply regularizer
    if (regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());
  }
  
protected:
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;
  
  double computeAlpha()
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

GradientBasedLearnerPtr GradientBasedLearner::createStochasticDescent(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return GradientBasedLearnerPtr(new GradientDescentLearner(learningRate, normalizeLearningRate));}

class BatchLearner : public GradientBasedLearner
{
public:
  BatchLearner(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr stoppingCriterion)
    : optimizer(optimizer), stoppingCriterion(stoppingCriterion) {}
    
  virtual void trainStochasticBegin()
  {
    Object::error("Batch::trainStochasticBegin", "This is not a stochastic learner");
    assert(false);
  }

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    Object::error("Batch::trainStochasticExample", "This is not a stochastic learner");
    assert(false);
  }
  
  virtual void trainStochasticEnd()
  {
    Object::error("Batch::trainStochasticEnd", "This is not a stochastic learner");
    assert(false);
  }

  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallback* progress)
  {
    //ProgressCallback silentCallback;
    if (progress)
      progress->progressStart("BatchLearner::trainBatch");
    FeatureGeneratorPtr params = parameters;
    if (!optimizer->optimize(objective, params, stoppingCriterion, progress))
      return false;
    parameters = params;
    if (progress)
      progress->progressEnd();
    return true;
  }
  
protected:
  VectorOptimizerPtr optimizer;
  OptimizerStoppingCriterionPtr stoppingCriterion;
};

GradientBasedLearnerPtr GradientBasedLearner::createBatch(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr stoppingCriterion)
  {return GradientBasedLearnerPtr(new BatchLearner(optimizer, stoppingCriterion));}

GradientBasedLearnerPtr GradientBasedLearner::createBatch(VectorOptimizerPtr optimizer, size_t maxIterations, double tolerance)
{
  OptimizerStoppingCriterionPtr stoppingCriterion;
  if (maxIterations > 0)
    stoppingCriterion = OptimizerStoppingCriterion::createMaxIterations(maxIterations);
  if (tolerance > 0)
  {
    OptimizerStoppingCriterionPtr c = OptimizerStoppingCriterion::createAverageImprovementThreshold(tolerance);
    stoppingCriterion = stoppingCriterion ? OptimizerStoppingCriterion::createOr(stoppingCriterion, c) : c;
  }
  assert(stoppingCriterion);
  return createBatch(optimizer, stoppingCriterion);
}

class NonLearnerGradientBasedLearner : public GradientBasedLearner
{
public:
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
    {}
    
  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallback* progress)
    {return true;}
};

GradientBasedLearnerPtr GradientBasedLearner::createNonLearner()
  {return new NonLearnerGradientBasedLearner();}
