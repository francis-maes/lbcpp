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
  
  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return bestFitness->getValue(0);}
  
  virtual string getDescription()
    {return T("Best fitness");}
  
protected:
  friend class SingleObjectiveSolverEvaluatorClass;
  
  FitnessPtr& bestFitness;
};

class HyperVolumeSolverEvaluator : public SolverEvaluator
{
public:
  HyperVolumeSolverEvaluator(ParetoFrontPtr& front = (*(ParetoFrontPtr*)0))
    : front(front) {}
  
  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return front->computeHyperVolume(front->getFitnessLimits()->getWorstPossibleFitness());}
  
  virtual string getDescription()
    {return T("Hypervolume");}
  
protected:
  friend class HyperVolumeSolverEvaluatorClass;
  
  ParetoFrontPtr& front;
};

class AdditiveEpsilonSolverEvaluator : public SolverEvaluator
{
public:
  AdditiveEpsilonSolverEvaluator(ParetoFrontPtr& front = (*(ParetoFrontPtr*)0), ParetoFrontPtr referenceFront = ParetoFrontPtr())
    : front(front), referenceFront(referenceFront) {}

  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return front->computeAdditiveEpsilonIndicator(referenceFront);}

  virtual string getDescription()
    {return T("Additive Epsilon");}

protected:
  friend class AdditiveEpsilonSolverEvaluatorClass;
  ParetoFrontPtr& front;
  ParetoFrontPtr referenceFront;
};

class MultiplicativeEpsilonSolverEvaluator : public SolverEvaluator
{
public:
  MultiplicativeEpsilonSolverEvaluator(ParetoFrontPtr& front = (*(ParetoFrontPtr*)0), ParetoFrontPtr referenceFront = ParetoFrontPtr())
    : front(front), referenceFront(referenceFront) {}

  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return front->computeMultiplicativeEpsilonIndicator(referenceFront);}

  virtual string getDescription()
    {return T("Additive Epsilon");}

protected:
  friend class MultiplicativeEpsilonSolverEvaluatorClass;
  
  ParetoFrontPtr& front;
  ParetoFrontPtr referenceFront;
};

class SpreadSolverEvaluator : public SolverEvaluator
{
public:
  SpreadSolverEvaluator(ParetoFrontPtr& front = (*(ParetoFrontPtr*)0)) : front(front) {}

  virtual double evaluateSolver(ExecutionContext& context, SolverPtr solver)
    {return front->computeSpreadIndicator();}

  virtual string getDescription()
    {return T("Spread");}

protected:
  friend class SpreadSolverEvaluatorClass;

  ParetoFrontPtr& front;
};

}; /* namespace lbcpp */

#endif // !ML_SOLVER_EVALUATORS_H_