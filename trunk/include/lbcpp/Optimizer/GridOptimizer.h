/*-----------------------------------------.---------------------------------.
| Filename: GridOptimizer.h                | Optimizer's base class for Grid |
| Author  : Arnaud Schoofs                 |                                 |
| Started : 11/03/2011 00:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_OPTIMIZER_H_
# define LBCPP_GRID_OPTIMIZER_H_

# include "../Core/Function.h"

namespace lbcpp
{

class GridOptimizerState : public Object {
public:
  
  virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const = 0;
  
protected:  
  friend class GridOptimizerStateClass;    
};
typedef ReferenceCountedObjectPtr<GridOptimizerState> GridOptimizerStatePtr;  
extern ClassPtr gridOptimizerStateClass;
  
  
// GridOptimizerState, Function, Function -> Variable
class GridOptimizer : public Function
{
public:
  virtual TypePtr getRequiredStateType() const
    {return gridOptimizerStateClass;}
  
  virtual TypePtr getRequiredFunctionForVariableType() const
    {return functionClass;}
  
  virtual TypePtr getRequiredFunctionForScoreType() const
    {return functionClass;}  
  
  virtual Variable optimize(ExecutionContext& context, const GridOptimizerStatePtr& state_, const FunctionPtr& getVariableFromTrace, const FunctionPtr& getScoreFromTrace) const = 0;
  
  // Function
  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
      switch (index) 
    {
      case 0:
        return getRequiredStateType();
      case 1:
        return getRequiredFunctionForVariableType();
      case 2:
        return getRequiredFunctionForScoreType();
      default:
        jassertfalse; return anyType;
    }
  }
  
  virtual String getOutputPostFix() const
    {return T("Optimized");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return variableType;}  // TODO arnaud : more precise needed ?
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {jassertfalse; return Variable();}
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return optimize(context, inputs[0].dynamicCast<GridOptimizerState>(), inputs[1].dynamicCast<Function>(), inputs[2].dynamicCast<Function>());}
  
};

typedef ReferenceCountedObjectPtr<GridOptimizer> GridOptimizerPtr;


}; /* namespace lbcpp */

#endif // !LBCPP_LBCPP_GRID_OPTIMIZER_H_
