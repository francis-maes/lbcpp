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
  ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(inputs[1].getObjectAndCast<IndependentMultiVariateDistribution>());
  generateNewWU(context, state, "test");
  return Variable(0); 
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}


void ProteinGridEvoOptimizer::generateNewWU(ExecutionContext& context, ProteinGridEvoOptimizerStatePtr state, const String& name) const
{
  WorkUnitPtr wu = new ProteinLearner();
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + state->sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name + T(".xml")));
}