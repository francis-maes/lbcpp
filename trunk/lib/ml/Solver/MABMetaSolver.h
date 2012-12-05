/*-----------------------------------------.---------------------------------.
| Filename: MABMetaSolver.h                | Multi-armed bandit based        |
| Author  : Francis Maes                   | Meta solver                     |
| Started : 16/09/2012 16:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_MAB_META_H_
# define ML_SOLVER_MAB_META_H_

# include <ml/Solver.h>
# include <ml/BanditPool.h>

namespace lbcpp
{

class MABMetaSolver : public IterativeSolver
{
public:
  MABMetaSolver(IterativeSolverPtr baseOptimizer, size_t numInstances, double explorationCoefficient, size_t numIterations = 0)
    : IterativeSolver(numIterations), baseOptimizer(baseOptimizer), numInstances(numInstances), explorationCoefficient(explorationCoefficient) {}
  MABMetaSolver() {}

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
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

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);

    FitnessLimitsPtr limits = problem->getFitnessLimits();
    pool = new BanditPool(new BPObjective(), explorationCoefficient);
    pool->reserveArms(numInstances);
    for (size_t i = 0; i < numInstances; ++i)
    {
      IterativeSolverPtr optimizer = baseOptimizer->cloneAndCast<IterativeSolver>();
      ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
      optimizer->startSolver(context, problem, compositeSolverCallback(callback, fillParetoFrontSolverCallback(front)), startingSolution);
      pool->createArm(new Pair(optimizer, front));
    }
  }

  virtual void stopSolver(ExecutionContext& context)
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
      optimizer->stopSolver(context);
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
      const PairPtr& pair = object.staticCast<Pair>();
      IterativeSolverPtr optimizer = pair->getFirst().staticCast<IterativeSolver>();
      ParetoFrontPtr front = pair->getSecond().staticCast<ParetoFront>();
      jassert(optimizer);
      double score = currentScore;
      bool shouldContinue = optimizer->iterateSolver(context, instanceIndex);
      currentScore = front->computeHyperVolume();
      if (!shouldContinue)
        return -DBL_MAX; // optimizer has converged, kill the arm
      else
        return currentScore > score ? 1.0 : 0.0;
    }

    double currentScore;
  };
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_MAB_META_H_
