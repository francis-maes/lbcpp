/*
 *  ProteinGridEvoOptimizer.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 2/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "precompiled.h"
#include "ProteinGridEvoOptimizer.h"

Variable ProteinGridEvoOptimizer::optimize(ExecutionContext& context, const FunctionPtr& function, const DistributionPtr& apriori, const Variable& guess) const
{
  /** 
   * Main idea :
   * if (state.xml exists) -> load state.xml
   * else -> new state (uses apriori distribution)
   * 
   * in state save number of evaluated results and number of generated results
   * in state save sorted pairs (map?) : id_wu <-> score
   *
   * if state.nbEvaluatedResults < threshold -> generate and/or wait for nbEvaluatedResults == threshold
   *
   * while( stopcriterion ) {
   *  select best X results and update state.distributions
   *  generate Y WUs, wait and get results
   * }
   *
   * Remarks:
   * synchronous : generate -> evaluate
   * asynchronous : sendWorkUnit -> extract score from trace file
   */
  
  // TODO arnaud : until now, not Grid but syncrhonous version for debug
  
  ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(apriori.dynamicCast<IndependentMultiVariateDistribution>());
  
  // generate
  while (state->getNumberGeneratedWU() < 10) {
    state->generateSampleWU(context, String((int) state->getNumberGeneratedWU()) + T(".xml"));
  }
  
  return Variable(0); 
}



void ProteinGridEvoOptimizerState::generateSampleWU(ExecutionContext& context, const String& name)
{
  WorkUnitPtr wu = new ProteinLearner();
  // TODO arnaud : set some parameters to 0 (disabled)
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name));
  numberGeneratedWU++;
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}