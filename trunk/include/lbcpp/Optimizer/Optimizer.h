/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizer <- Function           |
| Author  : Julien Becker                  |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Function/StoppingCriterion.h>

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

// Function, Function -> OptimizerState
class Optimizer : public Function
{
public:
  Optimizer(const File& optimizerStateFile = File::nonexistent)
    : optimizerStateFile(optimizerStateFile) {}

  /* Optimizer */
  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const = 0;

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const = 0;

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return optimizationProblemClass;}

  virtual String getOutputPostFix() const
    {return T("Optimized");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return optimizerStateClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  /* Object */
  virtual int __call(LuaState& state);

protected:
  friend class OptimizerClass;

  File optimizerStateFile;

  /* Optimizer */
  void saveOptimizerState(ExecutionContext& context, const OptimizerStatePtr& state) const;

  OptimizerStatePtr loadOptimizerState(ExecutionContext& context) const;
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

//extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples, bool verbose = false);  
extern OptimizerPtr edaOptimizer(size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(), double slowingFactor = 0, bool reinjectBest = false, bool verbose = false);
extern OptimizerPtr asyncEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(), double slowingFactor = 0, bool reinjectBest = false, bool verbose = false);

extern OptimizerPtr cmaesOptimizer(size_t numIterations);

extern OptimizerPtr bestFirstSearchOptimizer(const std::vector<StreamPtr>& streams, const File& optimizerStateFile = File::nonexistent);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
