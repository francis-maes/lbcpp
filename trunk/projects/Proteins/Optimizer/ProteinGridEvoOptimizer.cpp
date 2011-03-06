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
#include <map>

Variable ProteinGridEvoOptimizer::optimize(ExecutionContext& context, const FunctionPtr& objective, const Variable& apriori) const
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
    state->inProgressWUs.insert(String((int) state->getNumberGeneratedWU()) + T(".xml"));
  }
  
  // send WUs
  
  /*while (state->getNumberEvaluated() < 10) {
    juce::Thread::sleep(5000);
  }*/
  
  ObjectVectorPtr builders = objectVector(gaussianDistributionBuilderClass, 1);
  builders->setElement(0, gaussianDistributionBuilder());
  
  std::multimap<double, String> myMultiMap;
  for (int i = 0; i < 10; ++i) {
    // TODO arnaud : XmlSerialisation bug with createObject
    File file(String("/Users/arnaudschoofs/Proteins/traces/") + String(i) + T(".trace"));
    XmlImporter importer(context, file);
    if (!importer.isOpened())
    {jassertfalse;}
    if (!importer.enter("shared"))
    {jassertfalse;}
    double score = ((importer.getCurrentElement())->getNextElement()->getChildElementAllSubText(String("variable"), String("error"))).getDoubleValue();
    
    state->evaluatedWUs.insert(std::pair<double, String>(score, String(i)));
  }
  
  std::multimap<double, String>::reverse_iterator it;
  for ( it=state->evaluatedWUs.rbegin(); it != state->evaluatedWUs.rend(); it++ )
  {
    builders->getAndCast<DistributionBuilder>(0)->addElement(5.0);
    std::cout << (*it).first << " => " << (*it).second << std::endl;
  }
  builders->getAndCast<DistributionBuilder>(0)->build(context);
  
  return Variable(0); 
}



void ProteinGridEvoOptimizerState::generateSampleWU(ExecutionContext& context, const String& name)
{
  WorkUnitPtr wu = new ProteinLearner();
  // TODO arnaud : set some parameters to 0 (disabled)
  //wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 250"));
  wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name));
  numberGeneratedWU++;
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}