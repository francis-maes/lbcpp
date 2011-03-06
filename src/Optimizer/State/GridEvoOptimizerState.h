/*
 *  GridEvoOptimizerState.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 6/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef GRID_EVO_OPTIMIZER_STATE_H_
#define GRID_EVO_OPTIMIZER_STATE_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Core/Enumeration.h>
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include <lbcpp/Distribution/ContinuousDistribution.h>

// TODO arnaud

namespace lbcpp
{
  class GridEvoOptimizerState : public Object {
  public:
    GridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    GridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) 
    {
      this->distributions = distributions->cloneAndCast<IndependentMultiVariateDistribution>();
      // TODO arnaud : if state.xml loaded
      numberGeneratedWU = 0;
      numberEvaluatedWU = 0;
    }
    
    //NumericalProteinFeaturesParametersPtr sampleParameters() const;
    //void generateSampleWU(ExecutionContext& context, const String& name);
    
    size_t getNumberGeneratedWU() const 
      {return numberGeneratedWU;}
    
    size_t getNumberEvaluatedWU() const 
      {return numberEvaluatedWU;}
    
  protected:
    IndependentMultiVariateDistributionPtr distributions;
    
    size_t numberGeneratedWU;
    size_t numberEvaluatedWU;
    
    friend class GridEvoOptimizerStateClass;
    
  };
  typedef ReferenceCountedObjectPtr<GridEvoOptimizerState> GridEvoOptimizerStatePtr;
  
}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_STATE_H_