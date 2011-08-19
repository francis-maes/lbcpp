/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizer <- Function           |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
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

// OptimizerContext, OptimizerState -> Variable
class Optimizer : public Function
{
public:
  /* Optimizer */
  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const FunctionPtr& objectiveFunction, const FunctionPtr& validationFunction) const = 0;

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const = 0;

  /* Function */
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return functionClass;}

  virtual String getOutputPostFix() const
    {return T("Optimized");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return optimizerStateClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples, bool verbose = false);  
extern OptimizerPtr edaOptimizer(const SamplerPtr& sampler, size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(), double slowingFactor = 0, bool reinjectBest = false, bool verbose = false);
//extern OptimizerPtr asyncEDAOptimizer(size_t numIterations, size_t populationSize, size_t numBests, size_t numberEvaluationsInProgress, StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr(), double slowingFactor = 0, bool reinjectBest = false, bool verbose = false);

extern OptimizerPtr bestFirstSearchOptimizer(const ObjectPtr& initialState, const std::vector<StreamPtr>& streams);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
