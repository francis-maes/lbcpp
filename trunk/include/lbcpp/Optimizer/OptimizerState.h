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

class OptimizationProblem : public Object
{
public:
  OptimizationProblem(const FunctionPtr& objective, const Variable& initialGuess = Variable(), const SamplerPtr& sampler = SamplerPtr(), const FunctionPtr& validation = FunctionPtr())
    : objective(objective), initialGuess(initialGuess), sampler(sampler), validation(validation) {}
  OptimizationProblem() {}

  const FunctionPtr& getObjective() const
    {return objective;}
    
  const Variable& getInitialGuess() const
    {return initialGuess;}
    
  const SamplerPtr& getSampler() const
    {return sampler;}

  const FunctionPtr& getValidation() const
    {return validation;}
    
protected:
  friend class OptimizationProblemClass;
  
  FunctionPtr objective;
  Variable initialGuess;
  SamplerPtr sampler;
  FunctionPtr validation;
};

extern ClassPtr optimizationProblemClass;
typedef ReferenceCountedObjectPtr<OptimizationProblem> OptimizationProblemPtr;

class OptimizerState : public Object
{
public:
  const Variable& getBestSolution() const
    {return bestSolution;}

  double getBestScore() const
    {return bestScore;}

  void submitSolution(const Variable& solution, double score);
  Variable finishIteration(ExecutionContext& context, const OptimizationProblemPtr& problem, size_t iteration, double bestIterationScore, const Variable& bestIterationSolution);

protected:
  friend class OptimizerStateClass;

  Variable bestSolution;
  double bestScore;
};

typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;

extern ClassPtr optimizerStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
