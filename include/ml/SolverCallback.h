/*-----------------------------------------.---------------------------------.
 | Filename: SolverCallback.h               | Base class for solver callbacks |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 13/03/2013 18:30               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SOLVER_CALLBACK_H_
# define ML_SOLVER_CALLBACK_H_

# include "predeclarations.h"
# include <ml/RandomVariable.h>

namespace lbcpp
{

/*
 ** SolverEvaluator
 */
class SolverEvaluator : public Object
{
public:
  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver) = 0;
  virtual string getDescription() = 0;
};

extern SolverEvaluatorPtr singleObjectiveSolverEvaluator(FitnessPtr& bestFitness);
extern SolverEvaluatorPtr hyperVolumeSolverEvaluator(ParetoFrontPtr front);

/*
 ** SolverCallback
 */
class SolverCallback : public Object
{
public:
  virtual void solverStarted(ExecutionContext& context, SolverPtr solver) {}
  virtual void solutionEvaluated(ExecutionContext& context, SolverPtr solver, ObjectPtr object, FitnessPtr fitness) = 0;
  virtual void solverStopped(ExecutionContext& context, SolverPtr solver) {}
  
  virtual bool shouldStop()
  {return false;}
};

extern SolverCallbackPtr storeBestFitnessSolverCallback(FitnessPtr& bestFitness);
extern SolverCallbackPtr storeBestSolutionSolverCallback(ObjectPtr& bestSolution);
extern SolverCallbackPtr storeBestSolverCallback(ObjectPtr& bestSolution, FitnessPtr& bestFitness);

extern SolverCallbackPtr fillParetoFrontSolverCallback(ParetoFrontPtr front);
extern SolverCallbackPtr maxEvaluationsSolverCallback(size_t maxEvaluations);

extern SolverCallbackPtr compositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2);
extern SolverCallbackPtr compositeSolverCallback(SolverCallbackPtr callback1, SolverCallbackPtr callback2, SolverCallbackPtr callback3);

extern SolverCallbackPtr evaluationPeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  size_t evaluationPeriod);
extern SolverCallbackPtr timePeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  double evaluationPeriod);
extern SolverCallbackPtr logTimePeriodEvaluatorSolverCallback(SolverEvaluatorPtr solverEvaluator, IVectorPtr evaluations, DVectorPtr cpuTimes, DVectorPtr scores,  double evaluationPeriod, double factor);

extern SolverCallbackPtr aggregatorEvaluatorSolverCallback(SolverEvaluatorPtr evaluator, std::vector<ScalarVariableMeanAndVariancePtr>* data);

}; /* namespace lbcpp */

#endif // !ML_SOLVER_CALLBACK_H_