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
    juce::Thread::sleep(1000);
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
    //state->distributionsBuilders->getAndCast<DistributionBuilder>(0)->addElement(5.0);
    std::cout << (*it).first << " => " << (*it).second << std::endl;
  }
  //state->distributionsBuilders->getAndCast<DistributionBuilder>(0)->build(context);
  File f(String("/Users/arnaudschoofs/Proteins/traces/9.trace"));
  XmlImporter importer(context, f);
  juce::XmlElement* elmt = importer.getCurrentElement()->getChildByName(T("variable"))->getChildByName(T("node"))->getChildByName(T("result"));
  Variable v = importer.loadVariable(elmt, numericalProteinPredictorParametersClass);
  NumericalProteinPredictorParametersPtr test = v.getObjectAndCast<NumericalProteinPredictorParameters>();
  std::cout << test->featuresParameters->toString() << std::endl;
  
  IndependentMultiVariateDistributionBuilderPtr builder = new IndependentMultiVariateDistributionBuilder(numericalProteinFeaturesParametersClass);
  builder->setSubDistributionBuilder(0, new GaussianDistributionBuilder());  // TODO arnaud : integer gaussian distribution builder
  builder->setSubDistributionBuilder(1, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(2, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(3, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(4, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(5, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(6, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(7, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(8, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(9, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(10, new BernoulliDistributionBuilder());
  builder->setSubDistributionBuilder(11, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(12, new GaussianDistributionBuilder());
  builder->setSubDistributionBuilder(13, new GaussianDistributionBuilder());
  
  builder->addElement(test->featuresParameters);
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