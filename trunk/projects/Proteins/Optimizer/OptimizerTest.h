/*-----------------------------------------.---------------------------------.
| Filename: OptimizerTest.h                | WorkUnit used to test Optimizer |
| Author  : Arnaud Schoofs                 | (debug purpose)                 |
| Started : 02/03/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef OPTIMIZER_TEST_H_
#define OPTIMIZER_TEST_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Function/ScalarFunction.h>

# include "../WorkUnit/ProteinLearner.h"

// TODO arnaud : a enlever
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../../../src/Distribution/Builder/GaussianDistributionBuilder.h"
# include "../../../src/Distribution/Builder/BernoulliDistributionBuilder.h"
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkInterface.h>
# include "../../../projects/Examples/OptimizerTestBed.h"
# include <lbcpp/Function/ScalarVectorFunction.h>

//# include <lbcpp/Optimizer/GridOptimizer.h>
//# include "../Optimizer/ProteinGridEvoOptimizer.h"
//# include "../../../src/Optimizer/Optimizer/Grid/GridEvoOptimizer.h"
//# include "../Predictor/ProteinPredictorParameters.h"
# include "ProteinGridEvoOptimizer.h"
//# include "../../../src/Optimizer/Optimizer/UniformSampleAndPickBestOptimizer.h"
# include "../../../src/Optimizer/Optimizer/EDAOptimizer.h"
# include "../../../src/Optimizer/Optimizer/AsyncEDAOptimizer.h"
//# include "../../../src/Optimizer/Context/SynchroneousOptimizerContext.h"
//# include "../../../src/Optimizer/Context/MultiThreadsOptimizerContext.h"

namespace lbcpp
{

  
class DebugNetworkWorkUnit : public WorkUnit
{
  virtual Variable run(ExecutionContext& context)
  {
    String projectName(T("DebugNetwork3"));
    String source(T("arnaud@monster24"));
    String destination(T("notdefined@unknown"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    // Establish network connection
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
      {context.errorCallback(T("Not connected !"));}
    
    context.informationCallback(managerHostName, T("Connected !"));
    ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(context, client, source);
    client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));  

    if (!interface)
      context.errorCallback(T("Can't get interface !"));
    
    context.informationCallback(T("Sending WU ..."));

    WorkUnitPtr wu = new FunctionWorkUnit(squareFunction(), 2.0);
      
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);  
    String res = interface->pushWorkUnit(request);
    if (res == T("Error"))
    {
      context.errorCallback(T("Trouble - We didn't correclty receive the acknowledgement"));
    }
    context.resultCallback(T("WorkUnitIdentifier"), res);
    
    client->sendVariable(new CloseCommunicationNotification());
    client->stopClient();
    
    return Variable();
  }
};  
class AsyncEDAOptimizerExperience : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    String projectName(T("AsyncEDAOptimizerExperience2"));
    String source(T("localhost"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("localhost"));
    size_t managerPort = 1665;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    // initial sampler
    std::vector<SamplerPtr> samplers;
    samplers.reserve(14);
    for (size_t i = 0; i < 14; ++i)
    {
      SamplerPtr mySampler;
      switch (i) {
        case 0:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 15);
          break;
        case 1:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 2:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 3:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 4:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 5:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 6:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 15);
          break;
        case 7:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 8:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 9:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 10:
          mySampler = bernoulliSampler(0.5, 0.1, 0.9); // avoid prob = 0 or prob = 1
          break;
        case 11:
          mySampler = discretizeSampler(gaussianSampler(10, 5), 0, 25);
          break;
        case 12:
          mySampler = discretizeSampler(gaussianSampler(20, 10), 0, 55);
          break;
        case 13:
          mySampler = discretizeSampler(gaussianSampler(50, 20), 0, 110);
          break;
        default:
          jassertfalse;
      }
      
      samplers.push_back(mySampler);
    }
    SamplerPtr sampler = objectCompositeSampler(numericalProteinFeaturesParametersClass, samplers);
    
    
    // Optimizer
    OptimizerPtr optimizer = asyncEDAOptimizer(20, 500, 150, 750, 0.15);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 300000);
    SamplerBasedOptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler, 300);
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
  
};

class GridEvoOptimizerExperience : public WorkUnit  
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    String projectName(T("GridEvoOptimizerExperience"));
    String source(T("localhost"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("localhost"));
    size_t managerPort = 1665;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    // initial distribution
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
    distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,9));
    distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,9));
    distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
    distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,4));
    distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,100));
    distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,225));
    
    // Create initial state either from distri or from existing file
    ProteinGridEvoOptimizerStatePtr state = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("GridEvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>(); //new ProteinGridEvoOptimizerState(distributions);
    
    // variables used by EvoOptimizer
    size_t totalNumberWuRequested = 10000; // total number of WUs to evaluate
    size_t numberWuToUpdate = 500;       // min number of WUs evaluated needed to update distribution
    size_t numberWuInProgress = 750;      // number of WUs in progress (either in thread pool or in Boinc Network), should be <= totalNumberWuRequested. (for local optimizer should be a little bit higher than the number of CPUs)
    size_t ratioUsedForUpdate = 3;      // number of WUs used to calculate new distribution is numberWuToUpdate/ratioUsedForUpdate 
    size_t timeToSleep = 5*60;            // time to sleep between work generation and attemps to use finished results (avoid busy waiting)
    size_t updateFactor = 10;            // preponderance of new distri vs old distri (low value avoid too quick convergence)
    
    /**
     * GridEvoOptimizer
     */
    GridEvoOptimizerPtr optimizer = new GridEvoOptimizer(totalNumberWuRequested, numberWuToUpdate, numberWuInProgress, ratioUsedForUpdate, projectName, source, destination,
                                                         managerHostName, managerPort, requiredMemory, requiredTime, timeToSleep, updateFactor);
    return optimizer->optimize(context, state, new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
    
  }
  
};

  
class OptimizerTestBedWorkUnit : public WorkUnit 
{
  virtual Variable run(ExecutionContext& context)
  {
    /*
    std::vector<double> test;
    test.push_back(0.435579);
    LinearTransformation::Tosz(test);
    std::cout << "HERE : " << test[0] << std::endl;
    return Variable();
    */
    
    
    //OptimizerPtr optimizer = edaOptimizer(20, 100, 30, 0.2);
    OptimizerPtr optimizer = asyncEDAOptimizer(10, 100, 30, 50, 0.1);
    
    DenseDoubleVectorPtr coefs = new DenseDoubleVector(4, 0.0);
    coefs->setValue(0, 2.0);
    coefs->setValue(1, 4.0);
    coefs->setValue(2, -3.0);
    coefs->setValue(3, 0.0);

    FunctionPtr f = new RastriginFunction(coefs, -1);
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 1000);
    
    SamplerPtr sampler = independentDoubleVectorSampler(4, gaussianSampler(0.0, 5.0));
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler, 10);  
    
    return optimizer->compute(context, optimizerContext, optimizerState);
    
    
    /*
    std::vector<double> coefs;
    coefs.push_back(0.0);
    coefs.push_back(0.0);
    ScalarVectorFunctionPtr f = new EllipsoidalFunction(coefs, 0.0);
    DenseDoubleVectorPtr input = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, 2);
    input->setValue(0, 0.0);
    input->setValue(1, 0.0);
    double output;
    output = f->compute(context, input).toDouble();
    std::cout << output << std::endl;
    */
    return Variable();
    

  }
};
  
class OptimizerTest : public WorkUnit 
{
public:
  virtual Variable run(ExecutionContext& context)
  {
/*
    std::vector<double> coefs;
    coefs.push_back(4.0);
    coefs.push_back(3.0);
    ScalarVectorFunctionPtr f = new EllipsoidalFunction(coefs, 2.0);
    DenseDoubleVectorPtr input = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, 2);
    input->setValue(0, 1.0);
    input->setValue(1, -3.0);
    double output;
    output = f->compute(context, input).toDouble();
    std::cout << output << std::endl;
    
    return Variable();
  */
    // variables used by GridOptimizer
    String projectName(T("ProteinNewOptimizer"));
    String source(T("arnaud@monster24"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredCpus = 1;
    size_t requiredTime = 1;
    
    
    // initial distribution
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
    distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,9));
    distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,9));
    distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
    distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,4));
    distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,100));
    distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,225));
    
    std::vector<SamplerPtr> samplers;
    samplers.reserve(14);
    for (size_t i = 0; i < 14; ++i)
    {
      SamplerPtr mySampler;
      switch (i) {
        case 0:
          mySampler = discretizeSampler(gaussianSampler(1, 3), 0, 15);
          break;
        case 1:
          mySampler = discretizeSampler(gaussianSampler(3, 3), 0, 20);
          break;
        case 2:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 3:
          mySampler = discretizeSampler(gaussianSampler(3, 3), 0, 20);
          break;
        case 4:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 5:
          mySampler = discretizeSampler(gaussianSampler(3, 3), 0, 20);
          break;
        case 6:
          mySampler = discretizeSampler(gaussianSampler(2, 3), 0, 15);
          break;
        case 7:
          mySampler = discretizeSampler(gaussianSampler(3, 3), 0, 20);
          break;
        case 8:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 9:
          mySampler = discretizeSampler(gaussianSampler(5, 3), 0, 20);
          break;
        case 10:
          mySampler = bernoulliSampler(0.5, 0.1, 0.9); // avoid prob = 0 or prob = 1
          break;
        case 11:
          mySampler = discretizeSampler(gaussianSampler(15, 2), 0, 25);
          break;
        case 12:
          mySampler = discretizeSampler(gaussianSampler(15, 10), 0, 55);
          break;
        case 13:
          mySampler = discretizeSampler(gaussianSampler(50, 15), 0, 110);
          break;
        default:
          jassertfalse;
      }
      
      samplers.push_back(mySampler);
    }
    SamplerPtr sampler = objectCompositeSampler(numericalProteinFeaturesParametersClass, samplers);
    
    /*WorkUnitPtr wu = new FunctionWorkUnit(squareFunction(), 5.0);
    wu->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("MyWorkUnit.xml")));
    std::cout << "HERE HERE HERE HERE !!!!" << std::endl;*/
    /*Thread::sleep(1000);
    FunctionPtr fc = squareFunction();
    std::cout << fc->toString() << std::endl;
    WorkUnitPtr wu = new FunctionWorkUnit(fc, 5.0);
    std::cout << wu->toString() << std::endl;
    
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("DistributedOptimizerContext::getNetworkInterfaceAndConnect"), T("Not connected !"));
    }
    //context.informationCallback(managerHostName, T("Connected !")); TODO arnaud : useless ?
    ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    
    WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    
    //interface->pushWorkUnit(request);
    //interface->closeCommunication();
    
    std::cout << request->toString() << std::endl;
    std::cout << "----------" << std::endl;
    */
    
    
    /*OptimizerStatePtr state = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("optimizerState.xml"))).staticCast<OptimizerState>();
    state->initialize();
    std::cout << state->getTotalNumberOfEvaluations() << " VS " << state->getTotalNumberOfRequests() << std::endl;*/
    
    /*
    // TESTS OPTIMIZER
    //OptimizerPtr optimizer = uniformSampleAndPickBestOptimizer(100);
    OptimizerPtr optimizer = edaOptimizer(10, 20, 5, false, false);
    //OptimizerPtr optimizer = asyncEDAOptimizer(10, 100, 30, 30, 100);
    //OptimizerContextPtr optimizerContext = synchroneousOptimizerContext(context, squareFunction());
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, new ProteinLearnerObjectiveFunction());
    //OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, squareFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime);
    //DistributionBasedOptimizerStatePtr optimizerState = new DistributionBasedOptimizerState();
    SamplerBasedOptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);
    //optimizerState->setDistribution(new UniformDistribution(-5,5));    // TODO arnaud use constructor from library
    //optimizerState->setDistribution(new GaussianDistribution(10, 10000));  // TODO arnaud use constructor from library
    return optimizer->compute(context, optimizerContext, optimizerState);
    */
    
    /*OptimizerPtr optimizer = asyncEDAOptimizer(15, 1000, 300, 1500, 15);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 150000);
    OptimizerStatePtr optimizerState = new OptimizerState();
    optimizerState->setDistribution(distributions);
    
    return optimizer->compute(context, optimizerContext, optimizerState);
    */
    //return Variable();
    
    /*
    // initial distribution
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
    distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,9));
    distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,9));
    distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
    distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,4));
    distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,100));
    distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,225));
    */
    // Create initial state either from distri or from existing file
    //ProteinGridEvoOptimizerStatePtr state = /*new ProteinGridEvoOptimizerState(distributions);*/ Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>();
    /*
    // variables used by EvoOptimizer
    size_t totalNumberWuRequested = 10000; // total number of WUs to evaluate
    size_t numberWuToUpdate = 300;       // min number of WUs evaluated needed to update distribution
    size_t numberWuInProgress = 500;      // number of WUs in progress (either in thread pool or in Boinc Network), should be <= totalNumberWuRequested. (for local optimizer should be a little bit higher than the number of CPUs)
    size_t ratioUsedForUpdate = 3;      // number of WUs used to calculate new distribution is numberWuToUpdate/ratioUsedForUpdate 
    size_t timeToSleep = 5*60;            // time to sleep between work generation and attemps to use finished results (avoid busy waiting)
    size_t updateFactor = 1;            // preponderance of new distri vs old distri (low value avoid too quick convergence)
    
    // variables used by GridOptimizer
    String projectName(T("BoincProteinLearner1"));
    String source(T("arnaud@monster24"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredTime = 1;
    
    */
    /**
     * EvoOptimizer is the Local Evo Optimizer.
     * To use it you need:
     * - implement a GridEvoOptimizerState, i.e. WorkUnitPtr generateSampleWU(ExecutionContext& context). The state should be serializable !
     * - implement a ...GetVariableFromTraceFunction, i.e function : trace -> variable (to optimize) used for this evaluation
     * - implement a ...GetScoreFromTraceFunction, i.e function : trace -> score to optimize (== 1 - getScoreToMinize())
     *   NB : this function won't be necessary in the future as it is the same for every kind of trace !
     * 
     * Example : see ProteinGridEvoOptimizer.h
     */
    //EvoOptimizerPtr optimizer = new EvoOptimizer(totalNumberWuRequested, numberWuToUpdate, numberWuInProgress, ratioUsedForUpdate, timeToSleep, updateFactor);
    //return optimizer->optimize(context, state, new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
    
    
    /**
     * GridEvoOptimizer
     */
    //GridEvoOptimizerPtr optimizer = new GridEvoOptimizer(totalNumberWuRequested, numberWuToUpdate, numberWuInProgress, ratioUsedForUpdate, projectName, source, destination,
    //                                                     managerHostName, managerPort, requiredMemory, requiredTime, timeToSleep, updateFactor);
    //return optimizer->optimize(context, state, new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
    
    
    
    /**
     * Tests
     */
    /*
    ExecutionTracePtr trace = Object::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/MyTrace.xml"))).staticCast<ExecutionTrace>();
    FunctionPtr f1 = new ProteinGetScoreFromTraceFunction();
    std::cout << f1->compute(context, trace).toString() << std::endl;
    
    FunctionPtr f2 = new ProteinGetVariableFromTraceFunction();
    std::cout << f2->compute(context, trace).toString() << std::endl;
    */
    return Variable();
    
  }
protected:
  friend class OptimizerTestClass;
};

  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_
