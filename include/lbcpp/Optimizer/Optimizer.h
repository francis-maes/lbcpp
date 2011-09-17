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

// Function, Function -> OptimizerState
class Optimizer : public Function
{
public:
  Optimizer(const File& optimizerStateFile = File::nonexistent)
    : optimizerStateFile(optimizerStateFile) {}

  /* Optimizer */
  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizationProblemPtr& problem) const = 0;

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context, OptimizationProblemPtr problem) const = 0;

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
extern OptimizerPtr hooOptimizer(size_t numIterations, double nu, double rho);

extern OptimizerPtr bestFirstSearchOptimizer(const std::vector<StreamPtr>& streams, const File& optimizerStateFile = File::nonexistent);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
