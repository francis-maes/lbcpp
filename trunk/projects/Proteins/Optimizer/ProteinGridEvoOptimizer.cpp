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


ProteinGridEvoOptimizerState::ProteinGridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions) : GridEvoOptimizerState(distributions) 
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

WorkUnitPtr ProteinGridEvoOptimizerState::generateSampleWU(ExecutionContext& context) const
{
  WorkUnitPtr wu = new ProteinLearner();
  NumericalProteinFeaturesParametersPtr parameters = (distributions->sample(RandomGenerator::getInstance())).getObjectAndCast<NumericalProteinFeaturesParameters>();
  // TODO arnaud args for location of data
  wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical(") + parameters->toString() + T(",sgd)\" -t ss3 -n 1 -m 20"));

  // FOR DEBUG
  //wu->parseArguments(context, T("-s /Users/arnaudschoofs/Proteins/PDB30Boinc -i /Users/arnaudschoofs/Proteins/PDB30BoincInitialProteins/ -p \"numerical((1,3,5,3,5,3,2,3,5,5,True,15,15,50),sgd)\" -t ss3 -n 1 -m 10"));
  //wu->saveToFile(context, File(T("/Users/arnaudschoofs/Proteins/wu/") + name + T(".xml")));
  
  return wu;
}

double ProteinGridEvoOptimizer::getScoreFromTrace(ExecutionTracePtr trace) const
{
  // TODO arnaud : implement when bug serialization fixed
  jassertfalse; // not implemented
  return 0;
  
  // OLD
  /*
  ExecutionTracePtr trace = interface->getExecutionTrace(*it)->getExecutionTrace(context);
  File f(T("/Users/arnaudschoofs/Proteins/traces/") + (*it) + T(".trace"));
  trace->saveToFile(context,f);
  XmlImporter importer(context, f);
  if (!importer.isOpened())
  {jassertfalse;}
  if (!importer.enter("shared"))
  {jassertfalse;}
  double score = ((importer.getCurrentElement())->getNextElement()->getChildElementAllSubText(String("variable"), String("error"))).getDoubleValue();
  */
}

Variable ProteinGridEvoOptimizer::getVariableFromTrace(ExecutionTracePtr trace) const 
{
  // TODO arnaud : implement when bug serialization fixed
  jassertfalse; // not implemented
  return Variable();
  
  // OLD
  /*
  File f(T("/Users/arnaudschoofs/Proteins/traces/") + String((*it).second) + T(".trace"));
  XmlImporter importer(context, f);
  juce::XmlElement* elmt = importer.getCurrentElement()->getChildByName(T("variable"))->getChildByName(T("node"))->getChildByName(T("result"));
  Variable v = importer.loadVariable(elmt, numericalProteinPredictorParametersClass);
  NumericalProteinPredictorParametersPtr test = v.getObjectAndCast<NumericalProteinPredictorParameters>();  
  state->distributionsBuilder->addElement(test->featuresParameters);
  */
  
}



// OLD code
/*
Variable ProteinGridEvoOptimizer::optimize(ExecutionContext& context, const FunctionPtr& objective, const Variable& apriori) const
{
    
  // TODO arnaud : until now, not Grid but syncrhonous version for debug
  
  ProteinGridEvoOptimizerStatePtr state = new ProteinGridEvoOptimizerState(apriori.dynamicCast<IndependentMultiVariateDistribution>());
                        
  // Init generation and send : OK
  String projectName(T("BoincFirstStage"));
  String source(T("boincadm@boinc.run"));
  String destination(T("boincadm@boinc.run"));
  String managerHostName(T("monster24.montefiore.ulg.ac.be"));
  //size_t managerPort = 1664;
  //size_t requiredCpus = 1;
  //size_t requiredMemory = 2;
  //size_t requiredTime = 10;

  NetworkClientPtr client = blockingNetworkClient(context);
  if (!client->startClient(managerHostName, managerPort))
  {
    context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
    return false;
  }
  context.informationCallback(managerHostName, T("Connected !"));
  
  ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, source);
  interface->sendInterfaceClass();
  
  while (state->totalNumberGeneratedWUs < 1) {
    WorkUnitPtr wu = state->generateSampleWU(context, String((int) state->totalNumberGeneratedWUs));
    
    NetworkRequestPtr request = new NetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    String res = interface->pushWorkUnit(request);
    
    if (res == T("Error"))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Touble - We didn't correclty receive the acknowledgement"));
      interface->closeCommunication();
      return false;
    }
    
    state->totalNumberGeneratedWUs++;
    state->inProgressWUs.insert(res);
    
    //request->setIdentifier(res);
    context.resultCallback(T("WorkUnitIdentifier"), res);
    
    //File f = context.getFile(projectName + T(".") + request->getIdentifier() + T(".request"));
    //request->saveToFile(*context, f);
  }
  interface->closeCommunication();
  
  
  //state->inProgressWUs.insert(T("1299675529047"));
  //state->inProgressWUs.insert(T("1299675566221"));
  state->inProgressWUs.insert(T("1299672901904"));
  state->inProgressWUs.insert(T("1299672903018"));
  state->inProgressWUs.insert(T("1299672904089"));
  state->inProgressWUs.insert(T("1299672905175"));
  state->inProgressWUs.insert(T("1299672906244"));
  state->inProgressWUs.insert(T("1299672907319"));
  state->inProgressWUs.insert(T("1299672908395"));
  state->inProgressWUs.insert(T("1299672910479"));

  
  // handle finished WU's

  while (state->currentEvaluatedWUs.size() < 10) {
    juce::Thread::sleep(5000);
    
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
      return false;
    }
    context.informationCallback(managerHostName, T("Connected !"));
    
    ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    
    std::set<String>::iterator it;
    for(it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
    {
      
      if(interface->isFinished(*it))
      {
        ExecutionTracePtr trace = interface->getExecutionTrace(*it)->getExecutionTrace(context);
        File f(T("/Users/arnaudschoofs/Proteins/traces/") + (*it) + T(".trace"));
        trace->saveToFile(context,f);
        XmlImporter importer(context, f);
        if (!importer.isOpened())
          {jassertfalse;}
        if (!importer.enter("shared"))
          {jassertfalse;}
        double score = ((importer.getCurrentElement())->getNextElement()->getChildElementAllSubText(String("variable"), String("error"))).getDoubleValue();
        state->currentEvaluatedWUs.insert(std::pair<double, String>(score,*it));
        state->inProgressWUs.erase(it++);
      }
      else 
        ++it;
      
   
      //File f(String("/Users/arnaudschoofs/Proteins/traces/") + *it + T(".trace"));
      //if (f.existsAsFile())
      //{
      //  XmlImporter importer(context, f);
      //  if (!importer.isOpened())
      //    {jassertfalse;}
      //  if (!importer.enter("shared"))
      //    {jassertfalse;}
      //  double score = ((importer.getCurrentElement())->getNextElement()->getChildElementAllSubText(String("variable"), String("error"))).getDoubleValue();
      //  state->currentEvaluatedWUs.insert(std::pair<double, String>(score,*it));
      //  state->inProgressWUs.erase(it++);
      //}
      //else ++it;
  
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
   
  return Variable(0); 
}
*/