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
  
  distributionsBuilder = new IndependentMultiVariateDistributionBuilder(numericalProteinFeaturesParametersClass);
  distributionsBuilder->setSubDistributionBuilder(0, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(1, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(2, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(3, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(4, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(5, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(6, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(7, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(8, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(9, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(10, new BernoulliDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(11, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(12, new PositiveIntegerGaussianDistributionBuilder());
  distributionsBuilder->setSubDistributionBuilder(13, new PositiveIntegerGaussianDistributionBuilder());
  
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
  
  // Init generation and send
  
  NetworkClientPtr client = blockingNetworkClient(context);
  if (!client->startClient(T("monster24.montefiore.ulg.ac.be"), 1664));
  {
    context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
    return false;
  }
  context.informationCallback(T("monster24.montefiore.ulg.ac.be"), T("Connected !"));
  ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, T("GridEvoOptimizer"));
  interface->sendInterfaceClass();
  while (state->totalNumberGeneratedWUs < 10) {
    WorkUnitPtr wu = state->generateSampleWU(context, String((int) state->totalNumberGeneratedWUs));
    NetworkRequestPtr request = new NetworkRequest(context, T("BoincFirstStage"), T("GridEvoOptimizer"), T("boincadm@boinc.run"), wu, 1, 1, 1);
    String res = interface->pushWorkUnit(request);
    if (res == T("Error"))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Touble - We didn't correclty receive the acknowledgement"));
      break;
    }
    state->totalNumberGeneratedWUs++;
    state->inProgressWUs.insert(res);
    
    //request->setIdentifier(res);
    context.resultCallback(T("WorkUnitIdentifier"), res);
    
    //File f = context.getFile(projectName + T(".") + request->getIdentifier() + T(".request"));
    //request->saveToFile(*context, f);
  }
  interface->closeCommunication();
  
  /*
    
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
    
  // TODO arnaud : select best results, update distributions
  size_t nb = 0;
  std::multimap<double, String>::reverse_iterator it;
  for ( it=state->currentEvaluatedWUs.rbegin(); it != state->currentEvaluatedWUs.rend() && nb < 5; it++ )
  {
    File f(T("/Users/arnaudschoofs/Proteins/traces/") + String((*it).second) + T(".trace"));
    XmlImporter importer(context, f);
    juce::XmlElement* elmt = importer.getCurrentElement()->getChildByName(T("variable"))->getChildByName(T("node"))->getChildByName(T("result"));
    Variable v = importer.loadVariable(elmt, numericalProteinPredictorParametersClass);
    NumericalProteinPredictorParametersPtr test = v.getObjectAndCast<NumericalProteinPredictorParameters>();  
    state->distributionsBuilder->addElement(test->featuresParameters);
    std::cout << (*it).first << " => " << (*it).second << std::endl;
    nb++;
  }
  
  IndependentMultiVariateDistributionPtr newDistri = state->distributionsBuilder->build(context);
  std::cout << newDistri->sample(RandomGenerator::getInstance()).toString() << std::endl;
  
  */
   
  return Variable(0); 
}



WorkUnitPtr ProteinGridEvoOptimizerState::generateSampleWU(ExecutionContext& context, const String& name)
{
  WorkUnitPtr wu = new ProteinLearner();
  // TODO arnaud : set some parameters to 0 (disabled)
  //wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + sampleParameters()->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 10"));
  //wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name + T(".xml")));
  //totalNumberGeneratedWUs++;
  //inProgressWUs.insert(name);
  return wu;
}

NumericalProteinFeaturesParametersPtr ProteinGridEvoOptimizerState::sampleParameters() const 
{
  // TODO arnaud : set seed
  NumericalProteinFeaturesParametersPtr test = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  return test;
}