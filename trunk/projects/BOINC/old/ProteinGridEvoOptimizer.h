/*-----------------------------------------.---------------------------------.
| Filename: ProteinGridEvoOptimizer.h      | Optimizer using Evolutionary    |
| Author  : Arnaud Schoofs                 | Algorithm on Grid for Protein   |
| Started : 01/03/2011                     | project (deprecated)            |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef PROTEIN_GRID_EVO_OPTIMIZER_H_
#define PROTEIN_GRID_EVO_OPTIMIZER_H_

# include "../../../src/Optimizer/Optimizer/Grid/GridEvoOptimizer.h"
# include "../WorkUnit/ProteinLearner.h"
# include "../Evaluator/ProteinEvaluator.h"
# include "../Predictor/NumericalProteinPredictorParameters.h"
#include <map>
#include <set>

namespace lbcpp
{  
  
class ProteinGridEvoOptimizerState : public GridEvoOptimizerState
{
public:
  ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions)
    : GridEvoOptimizerState(distributions) {}
  ProteinGridEvoOptimizerState() {}
  
  virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const 
  {
    WorkUnitPtr wu = new ProteinLearner();
    NumericalProteinFeaturesParametersPtr parameters = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
    // TODO arnaud args for location of data
    wu->parseArguments(context, T("-s ./../../projects/boinc.run.montefiore.ulg.ac.be_evo/supervision -i ./../../projects/boinc.run.montefiore.ulg.ac.be_evo/predicted -p \"numerical(") + parameters->toString() + T(",sgd)\" -t ss3 -n 1"));
    return wu;
  }
  
protected:    
  friend class ProteinGridEvoOptimizerStateClass;

};
typedef ReferenceCountedObjectPtr<ProteinGridEvoOptimizerState> ProteinGridEvoOptimizerStatePtr;


class ProteinGetVariableFromTraceFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return executionTraceClass;}
  
  virtual String getOutputPostFix() const
    {return T("Variable to optimize extracted");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return variableType;}
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    std::vector<ExecutionTraceItemPtr> vec = (input.dynamicCast<ExecutionTrace>())->getRootNode()->getSubItems();
    ExecutionTraceNodePtr traceNode = vec[0].staticCast<ExecutionTraceNode>();
    if (!traceNode)
      return numericalProteinPredictorParameters();
    
    std::vector< std::pair<String, Variable> > results = traceNode->getResults();
    if (results.size() == 0)
      return numericalProteinPredictorParameters();
    
    if (results[0].second.dynamicCast<NumericalProteinPredictorParameters>())
      return results[0].second.dynamicCast<NumericalProteinPredictorParameters>()->featuresParameters;
    else
      return numericalProteinPredictorParameters();
  }    
};

typedef ReferenceCountedObjectPtr<ProteinGetVariableFromTraceFunction> ProteinGetVariableFromTraceFunctionPtr;

class ProteinGetScoreFromTraceFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return executionTraceClass;}
  
  virtual String getOutputPostFix() const
    {return T("Score extracted");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    std::vector<ExecutionTraceItemPtr> vec = (input.dynamicCast<ExecutionTrace>())->getRootNode()->getSubItems();    
  
    ExecutionTraceNodePtr traceNode = vec[0].dynamicCast<ExecutionTraceNode>();
    if (!traceNode)
      return 0;
    
    
    Variable returnValue = traceNode->getReturnValue();
    if (!returnValue.exists())
      return 0;
    
    if (returnValue.dynamicCast<ProteinLearnerScoreObject>())
      return 1 - (returnValue.dynamicCast<ProteinLearnerScoreObject>())->getScoreToMinimize();
    else 
      return 0;
    
  }  
};

typedef ReferenceCountedObjectPtr<ProteinGetVariableFromTraceFunction> ProteinGetVariableFromTraceFunctionPtr;


}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_