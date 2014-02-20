/*-----------------------------------------.---------------------------------.
| Filename: SolverCallbacks.h              | Solver Callbacks                |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2012 14:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLVER_CALLBACKS_H_
# define ML_SOLVER_CALLBACKS_H_

# include <ml/SolverCallback.h>
# include <ml/Fitness.h>
# include <ml/SolutionContainer.h>

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
  StoreBestFitnessSolverCallback(FitnessPtr& bestFitness) : res(bestFitness) {}
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
  StoreBestSolutionSolverCallback(ObjectPtr& bestSolution) : res(bestSolution) {}
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
  EvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores)
    : solverEvaluator(solverEvaluator), evaluations(evaluations), cpuTimes(cpuTimes), scores(scores), numEvaluations(0) {}
  EvaluatorSolverCallback() : numEvaluations(0) {}

  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
    {startTime = Time::getHighResolutionCounter(); numEvaluations = 0;}
  
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness) = 0;

protected:
  friend class EvaluatorSolverCallbackClass;

  SolverEvaluatorPtr solverEvaluator;
  IVectorPtr evaluations;
  DVectorPtr cpuTimes;
  DVectorPtr scores;

  double startTime;
  size_t numEvaluations;
  
  void appendResult(ExecutionContext& context, SolverPtr solver, size_t numEvaluations, double cpuTime, double score)
  {
    evaluations->append(numEvaluations);
    cpuTimes->append(cpuTime);
    scores->append(score);
    if (solver->getVerbosity() > verbosityQuiet)
      context.resultCallback(solverEvaluator->getDescription(), score);
  }
};

/**
 * Callback for evaluating a Solver every \f$n\f$ number of evaluations.
 **/
class EvaluationPeriodEvaluatorSolverCallback : public EvaluatorSolverCallback
{
public:
  EvaluationPeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  size_t evaluationPeriod)
    : EvaluatorSolverCallback(solverEvaluator, evaluations, cpuTimes, scores), evaluationPeriod(evaluationPeriod) {}
  EvaluationPeriodEvaluatorSolverCallback() : EvaluatorSolverCallback(), evaluationPeriod(0) {}
  
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    ++numEvaluations;
    if (numEvaluations % evaluationPeriod == 0)
      appendResult(context, solver, numEvaluations, Time::getHighResolutionCounter() - startTime, solverEvaluator->evaluateSolver(context, solver));
  }

protected:
  friend class EvaluationPeriodEvaluatorSolverCallbackClass;

  size_t evaluationPeriod;
};

class TimePeriodEvaluatorSolverCallback : public EvaluatorSolverCallback
{
public:
  TimePeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  double evaluationPeriod)
    : EvaluatorSolverCallback(solverEvaluator, evaluations, cpuTimes, scores), evaluationPeriod(evaluationPeriod), prevScore(DVector::missingValue) {}
  TimePeriodEvaluatorSolverCallback() : EvaluatorSolverCallback(), evaluationPeriod(0.0), prevScore(DVector::missingValue) {}
  
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
  {
    lastEvaluationTime = Time::getHighResolutionCounter();
    prevScore = DVector::missingValue;
    EvaluatorSolverCallback::solverStarted(context, solver);
  }
  
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    ++numEvaluations;
    double curTime = Time::getHighResolutionCounter();
    while (curTime - lastEvaluationTime >= evaluationPeriod)
    {
      appendResult(context, solver, numEvaluations, lastEvaluationTime - startTime, prevScore);
      lastEvaluationTime += evaluationPeriod;
    }
    prevScore = solverEvaluator->evaluateSolver(context, solver);
  }

protected:
  friend class TimePeriodEvaluatorSolverCallbackClass;

  double evaluationPeriod;
  double lastEvaluationTime;
  double prevScore;
};

class AggregatorEvaluatorSolverCallback : public SolverCallback
{
public:
  AggregatorEvaluatorSolverCallback(std::vector<SolverEvaluatorPtr> evaluators, std::map<string,std::vector<EvaluationPoint>>* data, size_t evaluationPeriod)
    : evaluators(evaluators), data(data), evaluationPeriod(evaluationPeriod) 
  {
    for (std::vector<SolverEvaluatorPtr>::iterator it = evaluators.begin(); it != evaluators.end(); ++it)
      (*data)[(*it)->getDescription()] = std::vector<EvaluationPoint>();
  }
  AggregatorEvaluatorSolverCallback() : evaluators(std::vector<SolverEvaluatorPtr>()), data(0), evaluationPeriod(1) {}
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver)
    { i = 0; numEvaluations = 0;}
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    ++numEvaluations;
    if (numEvaluations % evaluationPeriod == 0)
    {
      for (std::vector<SolverEvaluatorPtr>::iterator it = evaluators.begin(); it != evaluators.end(); ++it)
      {
        while ((*data)[(*it)->getDescription()].size() <= i)
          (*data)[(*it)->getDescription()].push_back(EvaluationPoint(numEvaluations));
        (*data)[(*it)->getDescription()][i].pushResult((*it)->evaluateSolver(context, solver));
      }
      ++i;
    }
  }

protected:
  friend class AggregatorEvaluatorSolverCallbackClass;
  std::vector<SolverEvaluatorPtr> evaluators;
  std::map<string,std::vector<EvaluationPoint>>* data;
  size_t i;
  size_t numEvaluations;
  size_t evaluationPeriod;
};

class LogTimePeriodEvaluatorSolverCallback : public TimePeriodEvaluatorSolverCallback
{
public:
  LogTimePeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  double evaluationPeriod, double factor)
    : TimePeriodEvaluatorSolverCallback(solverEvaluator, evaluations, cpuTimes, scores, evaluationPeriod), factor(factor) {}
  LogTimePeriodEvaluatorSolverCallback() : TimePeriodEvaluatorSolverCallback(), factor(0.0) {}
  
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness)
  {
    ++numEvaluations;
    double curTime = Time::getHighResolutionCounter();
    bool resultsAppended = (curTime - lastEvaluationTime >= evaluationPeriod);
    while (curTime - lastEvaluationTime >= evaluationPeriod)
    {
      double timeElapsed = lastEvaluationTime - startTime;
      appendResult(context, solver, numEvaluations, timeElapsed, prevScore);
      lastEvaluationTime += evaluationPeriod;
      evaluationPeriod *= factor;
    }
    if (resultsAppended)
      prevScore = solverEvaluator->evaluateSolver(context, solver);
  }

protected:
  friend class LogTimePeriodEvaluatorSolverCallbackClass;
  
  double factor;
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_CALLBACKS_H_
