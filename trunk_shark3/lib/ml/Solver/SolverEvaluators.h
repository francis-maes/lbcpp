/*-----------------------------------------.---------------------------------.
 | Filename: SolverEvaluators.h             | Solver Evaluators               |
 | Author  : Denny Verbeeck                 |                                 |
 | Started : 11/03/2013 16:13               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SOLVER_EVALUATORS_H_
# define ML_SOLVER_EVALUATORS_H_

# include <ml/Solver.h>

namespace lbcpp
{
  
class SingleObjectiveSolverEvaluator : public SolverEvaluator
{
public:
  SingleObjectiveSolverEvaluator(FitnessPtr& bestFitness = (*(FitnessPtr* )0))
    : bestFitness(bestFitness) {} 
  
  double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return bestFitness->getValue(0);}
  
protected:
  friend class SingleObjectiveSolverEvaluatorClass;
  
  FitnessPtr& bestFitness;
};

class HyperVolumeSolverEvaluator : public SolverEvaluator
{
public:
  HyperVolumeSolverEvaluator(ParetoFrontPtr front = ParetoFrontPtr())
    : front(front) {}
  
  double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return front->computeHyperVolume(front->getFitnessLimits()->getWorstPossibleFitness());}
  
protected:
  friend class HyperVolumeSolverEvaluatorClass;
  
  ParetoFrontPtr front;
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_EVALUATORS_H_