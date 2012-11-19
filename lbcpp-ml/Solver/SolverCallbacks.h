/*-----------------------------------------.---------------------------------.
| Filename: SolverCallbacks.h              | Solver Callbacks                |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2012 14:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLVER_CALLBACKS_H_
# define LBCPP_ML_SOLVER_CALLBACKS_H_

# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

class CompositeSolverCallback : public SolverCallback
{
public:
  CompositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2, SolverCallbackPtr callback3)
    : callbacks(3) {callbacks[0] = callback1; callbacks[1] = callback2; callbacks[2] = callback3;}
  CompositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2)
    : callbacks(2) {callbacks[0] = callback1; callbacks[1] = callback2;}
  CompositeSolverCallback() {}

  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->solverStarted(context, solver);
  }

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->solutionEvaluated(context, solver, object, fitness);
  }
  
  virtual void solverStopped(ExecutionContext& context, SolverPtr solver)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->solverStopped(context, solver);
  }
  
  virtual bool shouldStop()
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      if (callbacks[i]->shouldStop())
        return true;
    return false;
  }

protected:
  friend class CompositeSolverCallbackClass;

  std::vector<SolverCallbackPtr> callbacks;
};

class StoreBestFitnessSolverCallback : public SolverCallback
{
public:
  StoreBestFitnessSolverCallback(FitnessPtr& bestFitness) : res(res) {}
  StoreBestFitnessSolverCallback() : res(*(FitnessPtr* )0) {}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    if (!res || fitness->strictlyDominates(res))
      res = fitness;
  }

protected:
  FitnessPtr& res;
};

class StoreBestSolutionSolverCallback : public SolverCallback
{
public:
  StoreBestSolutionSolverCallback(ObjectPtr& bestSolution) : res(res) {}
  StoreBestSolutionSolverCallback() : res(*(ObjectPtr* )0) {}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    if (!bestFitness || fitness->strictlyDominates(bestFitness))
    {
      bestFitness = fitness;
      res = object;
    }
  }

protected:
  FitnessPtr bestFitness;
  ObjectPtr& res;
};

class StoreBestSolverCallback : public SolverCallback
{
public:
  StoreBestSolverCallback(ObjectPtr& bestSolution, FitnessPtr& bestFitness) : bestSolution(bestSolution), bestFitness(bestFitness) {}
  StoreBestSolverCallback() : bestSolution(*(ObjectPtr* )0), bestFitness(*(FitnessPtr* )0) {}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    if (!bestFitness || fitness->strictlyDominates(bestFitness))
    {
      bestFitness = fitness;
      bestSolution = object;
    }
  }

protected:
  ObjectPtr& bestSolution;
  FitnessPtr& bestFitness;
};

class FillParetoFrontSolverCallback : public SolverCallback
{
public:
  FillParetoFrontSolverCallback(ParetoFrontPtr front = ParetoFrontPtr())
    : front(front) {}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
    {front->insertSolution(object, fitness);}
  
  virtual void solverStopped(ExecutionContext& context, SolverPtr solver)
  {
    if (solver->getVerbosity() >= verbosityProgressAndResult)
      context.resultCallback("solutions", front);
  }

protected:
  friend class FillParetoFrontSolverCallbackClass;

  ParetoFrontPtr front;
};

class MaxEvaluationsSolverCallback : public SolverCallback
{
public:
  MaxEvaluationsSolverCallback(size_t maxEvaluations = 0)
    : maxEvaluations(maxEvaluations), numEvaluations(0) {}

  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
    {numEvaluations = 0;}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
    {++numEvaluations;}

  virtual bool shouldStop()
    {return numEvaluations >= maxEvaluations;}

protected:
  friend class MaxEvaluationsSolverCallbackClass;

  size_t maxEvaluations;
  size_t numEvaluations;
};

class EvaluatorSolverCallback : public SolverCallback
{
public:
  EvaluatorSolverCallback(size_t evaluationPeriod, DVectorPtr cpuTimes)
    : evaluationPeriod(evaluationPeriod), cpuTimes(cpuTimes), numEvaluations(0) {}
  EvaluatorSolverCallback() : evaluationPeriod(0), numEvaluations(0) {}
  
  virtual void evaluate(ExecutionContext& context, SolverPtr solver) = 0;

  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
  {
    numEvaluations = 0;
    startTime = Time::getHighResolutionCounter();
  }
  
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    ++numEvaluations;
    if (numEvaluations % evaluationPeriod == 0)
    {
      evaluate(context, solver);
      cpuTimes->append(Time::getHighResolutionCounter() - startTime);
    }
  }

protected:
  friend class EvaluatorSolverCallbackClass;

  size_t evaluationPeriod;
  DVectorPtr cpuTimes;

  size_t numEvaluations;
  double startTime;
};

class SingleObjectiveEvaluatorSolverCallback : public EvaluatorSolverCallback
{
public:
  SingleObjectiveEvaluatorSolverCallback(size_t evaluationPeriod, DVectorPtr cpuTimes, DVectorPtr scores)
    : EvaluatorSolverCallback(evaluationPeriod, cpuTimes), scores(scores) {}
  SingleObjectiveEvaluatorSolverCallback() {}
  
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
  {
    EvaluatorSolverCallback::solverStarted(context, solver);
    bestFitness = FitnessPtr();
  }

  virtual void evaluate(ExecutionContext& context, SolverPtr solver)
    {scores->append(bestFitness->getValue(0));}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    if (!bestFitness || fitness->strictlyDominates(bestFitness))
      bestFitness = fitness;
    EvaluatorSolverCallback::solutionEvaluated(context, solver, object, fitness);
  }

protected:
  friend class SingleObjectiveEvaluatorSolverCallbackClass;

  DVectorPtr scores;
  FitnessPtr bestFitness;
};

class HyperVolumeEvaluatorSolverCallback : public EvaluatorSolverCallback
{
public:
  HyperVolumeEvaluatorSolverCallback(size_t evaluationPeriod, DVectorPtr cpuTimes, DVectorPtr scores)
    : EvaluatorSolverCallback(evaluationPeriod, cpuTimes), scores(scores) {}
  HyperVolumeEvaluatorSolverCallback() {}
  
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
  {
    EvaluatorSolverCallback::solverStarted(context, solver);
    front = new ParetoFront();
  }

  virtual void evaluate(ExecutionContext& context, SolverPtr solver)
    {scores->append(front->computeHyperVolume(solver->getProblem()->getFitnessLimits()->getWorstPossibleFitness()));}

  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    front->insertSolution(object, fitness);
    EvaluatorSolverCallback::solutionEvaluated(context, solver, object, fitness);
  }

protected:
  friend class HyperVolumeEvaluatorSolverCallbackClass;

  DVectorPtr scores;
  ParetoFrontPtr front;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLVER_CALLBACKS_H_
