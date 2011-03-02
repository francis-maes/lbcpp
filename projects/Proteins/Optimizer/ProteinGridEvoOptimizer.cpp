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
  ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState();
  /*for (int i = 0; i < 5; ++i) {
    // TODO arnaud : wrap RunWorkUnit in a function
    WorkUnitPtr wu = new ProteinLearner();
    wu->parseArguments(context, "-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 20");
    wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + String(i) + T(".xml")));
  }*/
  return Variable(0); 
}