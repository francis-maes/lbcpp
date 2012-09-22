/*-----------------------------------------.---------------------------------.
| Filename: OptimizerState.h               | State associated with an        |
| Author  : Julien Becker                  | Optimizer (useful to restart    |
| Started : 22/08/2011 13:43               | the Optimizer)                  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_STATE_H_
# define LBCPP_OPTIMIZER_STATE_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class DecisionProblemState;
typedef ReferenceCountedObjectPtr<DecisionProblemState> DecisionProblemStatePtr;

class OptimizationProblem : public Object
{
public:
  OptimizationProblem(const FunctionPtr& objective, const Variable& initialGuess = Variable(), const SamplerPtr& sampler = SamplerPtr(), const FunctionPtr& validation = FunctionPtr())
    : objective(objective), initialGuess(initialGuess), sampler(sampler), validation(validation), maximisationProblem(false) {}

  TypePtr getSolutionsType() const
    {return objective->getRequiredInputType(0, 1);}

  const FunctionPtr& getObjective() const
    {return objective;}
    
  const Variable& getInitialGuess() const
    {return initialGuess;}
    
  const SamplerPtr& getSampler() const
    {return sampler;}

  const FunctionPtr& getValidation() const
    {return validation;}

  const DecisionProblemStatePtr& getInitialState() const
    {return initialState;}

  void setInitialState(const DecisionProblemStatePtr& state)
    {initialState = state;}
    
  bool isMaximisationProblem() const
    {return maximisationProblem;}
    
  void setMaximisationProblem(bool isMaximisationProblem)
    {maximisationProblem = isMaximisationProblem;}
    
protected:
  friend class OptimizationProblemClass;
  
  FunctionPtr objective;
  Variable initialGuess;
  SamplerPtr sampler;
  DecisionProblemStatePtr initialState;
  FunctionPtr validation;
  bool maximisationProblem;

  OptimizationProblem() {}
};

extern ClassPtr optimizationProblemClass;
typedef ReferenceCountedObjectPtr<OptimizationProblem> OptimizationProblemPtr;

class OptimizerState : public Object
{
public:
  OptimizerState(const OptimizationProblemPtr& problem) 
    : problem(problem), bestScore(getWorstScore()) {}
  OptimizerState() : bestScore(0.0) {}

  const Variable& getBestSolution() const
    {return bestSolution;}

  double getBestScore() const
    {return bestScore;}

  void submitSolution(const Variable& solution, double score);
  Variable finishIteration(ExecutionContext& context, size_t iteration, double bestIterationScore, const Variable& bestIterationSolution);

  double getWorstScore() const
    {return problem->isMaximisationProblem() ? -DBL_MAX : DBL_MAX;}

  bool isScoreBetterThan(double newScore, double previousScore)
    {return problem->isMaximisationProblem() ? (newScore > previousScore) : (newScore < previousScore);}

  const OptimizationProblemPtr& getProblem() const
    {return problem;}

  const FunctionPtr& getObjective() const
    {return problem->getObjective();}

protected:
  friend class OptimizerStateClass;

  OptimizationProblemPtr problem;
  Variable bestSolution;
  double bestScore;
};

typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;

extern ClassPtr optimizerStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
