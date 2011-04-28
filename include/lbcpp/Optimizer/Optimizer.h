/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizer <- Function           |
| Author  : Francis Maes, Arnaud Schoofs   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>

namespace lbcpp
{

// OptimizerContext, OptimizerState -> Variable
class Optimizer : public Function
{
public:
  virtual TypePtr getRequiredContextType() const;
  virtual TypePtr getRequiredStateType() const;
  virtual Variable optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const = 0;
  
  // Function
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual String getOutputPostFix() const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples, bool verbose = false);
extern OptimizerPtr edaOptimizer(size_t numIterations, size_t populationSize, size_t numBests, bool reinjectBest = false, bool verbose = false);
extern OptimizerPtr asyncEDAOptimizer(size_t totalNumberEvaluationsRequested, size_t numberEvaluationsToUpdate, size_t ratioUsedForUpdate, size_t timeToSleep, size_t updateFactor, size_t numberEvaluationsInProgress, bool verbose = false);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
