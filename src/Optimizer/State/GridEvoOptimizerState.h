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
# include "../../Distribution/Builder/IndependentMultiVariateDistributionBuilder.h"
# include <lbcpp/Distribution/ContinuousDistribution.h>

namespace lbcpp
{
  class GridEvoOptimizerState : public Object {
  public:
    GridEvoOptimizerState()
      {jassert(false);} // TODO arnaud
    GridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : distributions(distributions)
    {
      // TODO arnaud : clone init distribution ?
      totalNumberGeneratedWUs = 0;
      totalNumberEvaluatedWUs = 0;
    }
    
    virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const = 0;
    
  protected:
    size_t totalNumberGeneratedWUs;
    size_t totalNumberEvaluatedWUs;
    
    std::vector<String> inProgressWUs;
    std::multimap<double, String> currentEvaluatedWUs;
    
    IndependentMultiVariateDistributionPtr distributions;
    IndependentMultiVariateDistributionBuilderPtr distributionsBuilder;
    
    friend class GridEvoOptimizerStateClass;
    friend class GridEvoOptimizer;
    friend class GridEvoOptimizerClass; // TODO arnaud necessary ?
    
  };
  typedef ReferenceCountedObjectPtr<GridEvoOptimizerState> GridEvoOptimizerStatePtr;
  
}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_STATE_H_