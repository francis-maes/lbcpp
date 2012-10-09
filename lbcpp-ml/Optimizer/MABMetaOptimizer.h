/*-----------------------------------------.---------------------------------.
| Filename: MABMetaOptimizer.h             | Multi-armed bandit based        |
| Author  : Francis Maes                   | Meta optimizer                  |
| Started : 16/09/2012 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OPTIMIZER_MAB_META_H_
# define LBCPP_ML_OPTIMIZER_MAB_META_H_

# include <lbcpp-ml/Optimizer.h>
# include <lbcpp/Optimizer/BanditPool.h>

namespace lbcpp
{

class MABMetaOptimizer : public IterativeOptimizer
{
public:
  MABMetaOptimizer(IterativeOptimizerPtr baseOptimizer, size_t numInstances, double explorationCoefficient, size_t numIterations = 0)
    : IterativeOptimizer(numIterations), baseOptimizer(baseOptimizer), numInstances(numInstances), explorationCoefficient(explorationCoefficient) {}
  MABMetaOptimizer() {}

  virtual bool iteration(ExecutionContext& context, size_t iter)
  {
    jassert(pool);
    size_t armIndex = pool->selectAndPlayArm(context);
    if (verbosity >= verbosityDetailed)
    {
      context.resultCallback("armIndex", armIndex);
      context.resultCallback("armObjective", pool->getArmMeanObjective(armIndex));
      context.resultCallback("armReward", pool->getArmMeanReward(armIndex));
      context.resultCallback("armPlayCount", pool->getArmPlayedCount(armIndex));
    }
    return true;
  }

  virtual void configure(ExecutionContext& context, ProblemPtr problem, ParetoFrontPtr front, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeOptimizer::configure(context, problem, front, initialSolution, verbosity);

    FitnessLimitsPtr limits = problem->getFitnessLimits();
    pool = new BanditPool(new Objective(), explorationCoefficient);
    pool->reserveArms(numInstances);
    for (size_t i = 0; i < numInstances; ++i)
    {
      IterativeOptimizerPtr optimizer = baseOptimizer->cloneAndCast<IterativeOptimizer>();
      optimizer->configure(context, problem, front, initialSolution, verbosityQuiet);
      pool->createArm(optimizer);
    }
  }

  virtual void clear(ExecutionContext& context)
  {
    if (verbosity >= verbosityDetailed)
    {
      pool->displayInformation(context, numInstances >= 3 ? numInstances / 3 : 1, 1);
      context.enterScope("arms");
      pool->displayAllArms(context);
      context.leaveScope();
    }
    for (size_t i = 0; i < numInstances; ++i)
    {
      IterativeOptimizerPtr optimizer = pool->getArmParameter(i).getObjectAndCast<IterativeOptimizer>();
      jassert(optimizer);
      optimizer->clear(context);
    }
    pool = BanditPoolPtr();
  }

protected:
  friend class MABMetaOptimizerClass;

  IterativeOptimizerPtr baseOptimizer;
  size_t numInstances;
  double explorationCoefficient;

  BanditPoolPtr pool;

  struct Objective : public BanditPoolObjective
  {
    Objective() : currentScore(-DBL_MAX) {}

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0.0; best = 1.0;}

    virtual double computeObjective(ExecutionContext& context, const Variable& parameter, size_t instanceIndex)
    {
      IterativeOptimizerPtr optimizer = parameter.getObjectAndCast<IterativeOptimizer>();
      jassert(optimizer);
      double score = currentScore;
      bool shouldContinue = optimizer->iteration(context, instanceIndex);
      currentScore = optimizer->computeHyperVolume();
      if (!shouldContinue)
        return -DBL_MAX; // optimizer has converged, kill the arm
      else
        return currentScore > score ? 1.0 : 0.0;
    }

    double currentScore;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OPTIMIZER_MAB_META_H_
