/*
 *  ProteinGridEvoOptimizer.cpp
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 2/03/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ProteinGridEvoOptimizer.h"

Variable ProteinGridEvoOptimizer::computeFunction(ExecutionContext& context, const Variable* inputs) const
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
   */
  
  ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(inputs[1].getObjectAndCast<IndependentMultiVariateDistribution>());
  generateSampleWU(context, state, "test");
  return Variable(0); 
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}


void ProteinGridEvoOptimizer::generateSampleWU(ExecutionContext& context, ProteinGridEvoOptimizerStatePtr state, const String& name) const
{
  WorkUnitPtr wu = new ProteinLearner();
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + state->sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name + T(".xml")));
}