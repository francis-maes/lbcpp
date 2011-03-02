/*-----------------------------------------.---------------------------------.
 | Filename: ProteinGridEvoOptimizer.h      | Optimizer using Evolutionary    |
 | Author  : Arnaud Schoofs                 | Algorithm on Grid for Protein   |
 | Started : 01/03/2010 23:45               | project                         |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef PROTEIN_GRID_EVO_OPTIMIZER_H_
#define PROTEIN_GRID_EVO_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include "../../../src/Optimizer/Optimizer/GridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../WorkUnit/ProteinLearner.h"

namespace lbcpp
{
  
  class ProteinGridEvoOptimizer : public GridEvoOptimizer
  {
  public:
    virtual size_t getMaximumNumRequiredInputs() const
    {return 2;} // do not use initial guess
    
    virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {
      switch (index) 
      {
        case 0:
          return (TypePtr) objectiveFunctionClass;
        case 1:
          return (TypePtr) independentMultiVariateDistributionClass(doubleType);
        default:
          jassert(false); // TODO arnaud
          return anyType;
      }
    }
    
  protected:
    virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {
      
      for (int i = 0; i < 5; ++i) {
        // TODO arnaud : wrap RunWorkUnit in a function
        WorkUnitPtr wu = new ProteinLearner();
        wu->parseArguments(context, "-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 20");
        wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + String(i) + T(".xml")));
      }
      return Variable(0); 
    }
    
    friend class ProteinGridEvoOptimizerClass;
    
  };  
  

}; /* namespace lbcpp */

#endif // !PROTEIN_GRID_EVO_OPTIMIZER_H_