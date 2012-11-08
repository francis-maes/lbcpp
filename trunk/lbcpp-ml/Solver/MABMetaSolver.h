/*-----------------------------------------.---------------------------------.
| Filename: MABMetaSolver.h                | Multi-armed bandit based        |
| Author  : Francis Maes                   | Meta solver                     |
| Started : 16/09/2012 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_MAB_META_H_
# define LBCPP_ML_SOLVER_MAB_META_H_

# include <lbcpp-ml/Solver.h>
# include <lbcpp-ml/BanditPool.h>

namespace lbcpp
{

class MABMetaSolver : public IterativeSolver
{
public:
  MABMetaSolver(IterativeSolverPtr baseOptimizer, size_t numInstances, double explorationCoefficient, size_t numIterations = 0)
    : IterativeSolver(numIterations), baseOptimizer(baseOptimizer), numInstances(numInstances), explorationCoefficient(explorationCoefficient) {}
  MABMetaSolver() {}

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

  virtual void configure(ExecutionContext& context, ProblemPtr problem, SolutionContainerPtr solutions, ObjectPtr initialSolution, Verbosity verbosity)
  {
    IterativeSolver::configure(context, problem, solutions, initialSolution, verbosity);

    FitnessLimitsPtr limits = problem->getFitnessLimits();
    pool = new BanditPool(new BPObjective(), explorationCoefficient);
    pool->reserveArms(numInstances);
    for (size_t i = 0; i < numInstances; ++i)
    {
      IterativeSolverPtr optimizer = baseOptimizer->cloneAndCast<IterativeSolver>();
      optimizer->configure(context, problem, solutions, initialSolution, verbosityQuiet);
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
      IterativeSolverPtr optimizer = pool->getArmObject(i).staticCast<IterativeSolver>();
      jassert(optimizer);
      optimizer->clear(context);
    }
    pool = BanditPoolPtr();
  }

protected:
  friend class MABMetaSolverClass;

  IterativeSolverPtr baseOptimizer;
  size_t numInstances;
  double explorationCoefficient;

  BanditPoolPtr pool;

  struct BPObjective : public StochasticObjective
  {
    BPObjective() : currentScore(-DBL_MAX) {}

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0.0; best = 1.0;}

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object, size_t instanceIndex)
    {
      IterativeSolverPtr optimizer = object.staticCast<IterativeSolver>();
      jassert(optimizer);
      double score = currentScore;
      bool shouldContinue = optimizer->iteration(context, instanceIndex);
      currentScore = optimizer->getSolutions().staticCast<ParetoFront>()->computeHyperVolume();
      if (!shouldContinue)
        return -DBL_MAX; // optimizer has converged, kill the arm
      else
        return currentScore > score ? 1.0 : 0.0;
    }

    double currentScore;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_MAB_META_H_
