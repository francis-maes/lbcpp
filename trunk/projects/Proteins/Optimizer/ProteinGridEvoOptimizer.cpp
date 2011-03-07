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


ProteinGridEvoOptimizerState::ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : distributions(distributions) 
{
  // TODO arnaud : if state.xml loaded
  totalNumberGeneratedWUs = 0;
  totalNumberEvaluatedWUs = 0;
  distributionsBuilders = objectVector(distributionBuilderClass, 14);
  distributionsBuilders->setElement(0, gaussianDistributionBuilder());
  distributionsBuilders->setElement(1, gaussianDistributionBuilder());
  distributionsBuilders->setElement(2, gaussianDistributionBuilder());
  distributionsBuilders->setElement(3, gaussianDistributionBuilder());
  distributionsBuilders->setElement(4, gaussianDistributionBuilder());
  distributionsBuilders->setElement(5, gaussianDistributionBuilder());
  distributionsBuilders->setElement(6, gaussianDistributionBuilder());
  distributionsBuilders->setElement(7, gaussianDistributionBuilder());
  distributionsBuilders->setElement(8, gaussianDistributionBuilder());
  distributionsBuilders->setElement(9, gaussianDistributionBuilder());
  distributionsBuilders->setElement(10, bernoulliDistributionBuilder());
  distributionsBuilders->setElement(11, gaussianDistributionBuilder());
  distributionsBuilders->setElement(12, gaussianDistributionBuilder());
  distributionsBuilders->setElement(13, gaussianDistributionBuilder());
}

void ProteinGridEvoOptimizerState::clearBuilders()
{
  for (size_t i = 0; i < distributionsBuilders->getNumElements(); ++i) {
    distributionsBuilders->getAndCast<DistributionBuilder>(i)->clear();
  }
}

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
  
  // Init generation
  while (state->totalNumberGeneratedWUs < 10) {
    state->generateSampleWU(context, String((int) state->totalNumberGeneratedWUs));
  }
    
  // handle finished WU's
  while (state->currentEvaluatedWUs.size() < 10) {
    juce::Thread::sleep(5000);
    std::set<String>::iterator it;
    for(it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
    {
      File f(String("/Users/arnaudschoofs/Proteins/traces/") + *it + T(".trace"));
      if (f.existsAsFile())
      {
        XmlImporter importer(context, f);
        if (!importer.isOpened())
          {jassertfalse;}
        if (!importer.enter("shared"))
          {jassertfalse;}
        double score = ((importer.getCurrentElement())->getNextElement()->getChildElementAllSubText(String("variable"), String("error"))).getDoubleValue();
        state->currentEvaluatedWUs.insert(std::pair<double, String>(score,*it));
        state->inProgressWUs.erase(it++);
      }
      else ++it;
    }
  }
    
  // TODO arnaud : select best results, update distributions, builder for IntegerGaussian
  std::multimap<double, String>::reverse_iterator it;
  for ( it=state->currentEvaluatedWUs.rbegin(); it != state->currentEvaluatedWUs.rend(); it++ )
  {
    state->distributionsBuilders->getAndCast<DistributionBuilder>(0)->addElement(5.0);
    std::cout << (*it).first << " => " << (*it).second << std::endl;
  }
  state->distributionsBuilders->getAndCast<DistributionBuilder>(0)->build(context);
  
  return Variable(0); 
}



void ProteinGridEvoOptimizerState::generateSampleWU(ExecutionContext& context, const String& name)
{
  WorkUnitPtr wu = new ProteinLearner();
  // TODO arnaud : set some parameters to 0 (disabled)
  //wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 10"));
  wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name + T(".xml")));
  totalNumberGeneratedWUs++;
  inProgressWUs.insert(name);
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}