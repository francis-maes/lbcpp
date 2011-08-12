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
# include "../WorkUnit/ProteinLearner.h"
# include "../../../projects/Examples/OptimizerTestBed.h"

#include <map>

namespace lbcpp
{

/*
 ** Experience that gave the graph on the protein problem in my thesis
 */
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
    OptimizerPtr optimizer = asyncEDAOptimizer(20, 500, 150, 750, StoppingCriterionPtr(), 0.15);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 300000);
    SamplerBasedOptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler, 300);
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
};

/*
 ** Experience that uses the state obtained after the 1st experience
 ** (stddev have benn increased)
 */  
class AsyncEDAOptimizerExperience3 : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    String projectName(T("AsyncEDAOptimizerExperience3"));
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
      // means obtained from first experience
      SamplerPtr mySampler;
      switch (i) {
        case 0:
          mySampler = discretizeSampler(gaussianSampler(3.42, 1), 0, 15);
          break;
        case 1:
          mySampler = discretizeSampler(gaussianSampler(4.467, 3), 0, 20);
          break;
        case 2:
          mySampler = discretizeSampler(gaussianSampler(8.513, 2), 0, 20);
          break;
        case 3:
          mySampler = discretizeSampler(gaussianSampler(4.58, 3), 0, 20);
          break;
        case 4:
          mySampler = discretizeSampler(gaussianSampler(12.07, 3), 0, 20);
          break;
        case 5:
          mySampler = discretizeSampler(gaussianSampler(4.66, 3), 0, 20);
          break;
        case 6:
          mySampler = discretizeSampler(gaussianSampler(1.58, 2), 0, 15);
          break;
        case 7:
          mySampler = discretizeSampler(gaussianSampler(4.447, 3), 0, 20);
          break;
        case 8:
          mySampler = discretizeSampler(gaussianSampler(6.333, 3), 0, 20);
          break;
        case 9:
          mySampler = discretizeSampler(gaussianSampler(6.807, 3), 0, 20);
          break;
        case 10:
          mySampler = bernoulliSampler(0.95, 0.05, 0.95); // avoid prob = 0 or prob = 1
          break;
        case 11:
          mySampler = discretizeSampler(gaussianSampler(9.16, 2), 0, 25);
          break;
        case 12:
          mySampler = discretizeSampler(gaussianSampler(16.38, 8), 0, 55);
          break;
        case 13:
          mySampler = discretizeSampler(gaussianSampler(56.6, 20), 0, 110);
          break;
        default:
          jassertfalse;
      }
      
      samplers.push_back(mySampler);
    }
    SamplerPtr sampler = objectCompositeSampler(numericalProteinFeaturesParametersClass, samplers);
    
    
    // Optimizer
    OptimizerPtr optimizer = asyncEDAOptimizer(20, 500, 150, 500, StoppingCriterionPtr(), 0.10);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 300000);
    SamplerBasedOptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler, 300);
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
};

/*
 ** Experience that uses the state obtained after the 2nd experience
 ** (stddev have benn increased)
 */  
class AsyncEDAOptimizerExperience4 : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    String projectName(T("AsyncEDAOptimizerExperience6"));
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
      // means obtained from first experience
      SamplerPtr mySampler;
      switch (i) {
        case 0:
          mySampler = discretizeSampler(gaussianSampler(3.613, 0.974), 0, 15);
          break;
        case 1:
          mySampler = discretizeSampler(gaussianSampler(3.62, 3.958), 0, 20);
          break;
        case 2:
          mySampler = discretizeSampler(gaussianSampler(9.113, 1.644), 0, 20);
          break;
        case 3:
          mySampler = discretizeSampler(gaussianSampler(4.153, 4.882), 0, 20);
          break;
        case 4:
          mySampler = discretizeSampler(gaussianSampler(17.48, 1.238), 0, 25);
          break;
        case 5:
          mySampler = discretizeSampler(gaussianSampler(5.34, 4.606), 0, 20);
          break;
        case 6:
          mySampler = discretizeSampler(gaussianSampler(1.933, 2.224), 0, 15);
          break;
        case 7:
          mySampler = discretizeSampler(gaussianSampler(5.3, 4.434), 0, 20);
          break;
        case 8:
          mySampler = discretizeSampler(gaussianSampler(7.5, 2.258), 0, 20);
          break;
        case 9:
          mySampler = discretizeSampler(gaussianSampler(9.113, 2.418), 0, 20);
          break;
        case 10:
          mySampler = bernoulliSampler(0.95, 0.00, 1.00); // avoid prob = 0 or prob = 1
          break;
        case 11:
          mySampler = discretizeSampler(gaussianSampler(9.0, 1.0), 0, 25);
          break;
        case 12:
          mySampler = discretizeSampler(gaussianSampler(16.91, 2.702), 0, 55);
          break;
        case 13:
          mySampler = discretizeSampler(gaussianSampler(97.16, 17.288), 0, 150);
          break;
        default:
          jassertfalse;
      }
      
      samplers.push_back(mySampler);
    }
    SamplerPtr sampler = objectCompositeSampler(numericalProteinFeaturesParametersClass, samplers);
    
    
    // Optimizer
    OptimizerPtr optimizer = asyncEDAOptimizer(20, 500, 150, 500, StoppingCriterionPtr(), 0.10);
    OptimizerContextPtr optimizerContext = distributedOptimizerContext(context, new ProteinLearnerObjectiveFunction(), projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime, 300000);
    SamplerBasedOptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler, 300);
    return optimizer->compute(context, optimizerContext, optimizerState);
  }
};
  
/*
 ** WorkUnit used to extract mean score at each iteration on Monster24
 */
class ExtractMeanScoreWorkUnit : public WorkUnit
{
  virtual Variable run(ExecutionContext& context)
  {
    juce::DirectoryIterator iter (File ("/home/arnaud/Manager2/AsyncEDAOptimizerExperience2"), true, "*.archive");
    std::map<juce::int64, double> scores; // map is sorted by identifier, i.e. by creation time !
    while (iter.next())
    {
      File theFileItFound (iter.getFile());
      jassertfalse;
//      NetworkArchivePtr archive = Object::createFromFile(context, theFileItFound).staticCast<NetworkArchive>();
//      ExecutionTracePtr trace =  archive->getExecutionTraceNetworkResponse()->getExecutionTrace(context);
      std::vector<ExecutionTraceItemPtr> vec;// = trace->getRootNode()->getSubItems();      
      Variable ret = (vec[0].dynamicCast<ExecutionTraceNode>())->getReturnValue();
      double validationScore = ret.toDouble();
      //std::cout << theFileItFound.getFileNameWithoutExtension().getLargeIntValue() << " " << validationScore << std::endl;
      scores.insert(std::pair<juce::int64, double>(theFileItFound.getFileNameWithoutExtension().getLargeIntValue(), validationScore));
    }

    double validationMean = 0.0;
    size_t nb = 0;
    size_t i = 0;
    size_t iteration = 1;
    std::map<juce::int64, double>::iterator it;
    for ( it=scores.begin() ; it != scores.end(); it++ )
    {
      if (it->second <= 1)  // score >= 1 indicates an error in this context (-> doesn't take them into account)
      {
        validationMean += it->second;
        nb++;
      }
      i++;
      if (i == 500) 
      {
        std::cout << iteration << " " << validationMean/nb << std::endl;
        validationMean = 0.0;
        i = 0;
        nb = 0;
        iteration++;
      }
    }
    return Variable();
  }
  
};
    
  

class OptimizerTestBedWorkUnit : public WorkUnit 
{
  virtual Variable run(ExecutionContext& context)
  {
    // dimension and coefs used for the OptimizerTestBed Function
    DenseDoubleVectorPtr coefs = new DenseDoubleVector(5, 0.0);
    coefs->setValue(0, 0.0);
    coefs->setValue(1, 0.0);
    coefs->setValue(2, 0.0);
    coefs->setValue(3, 0.0);
    coefs->setValue(4, 0.0);

    double fopt = 0.0;
    
    FunctionPtr f1 = new SphereFunction(coefs, fopt);
    FunctionPtr f2 = new EllipsoidalFunction(coefs, fopt);
    FunctionPtr f3 = new RastriginFunction(coefs, fopt);
    FunctionPtr f4 = new BucheRastriginFunction(coefs, fopt);
    FunctionPtr f5 = new LinearSlopeFunction(coefs, fopt);
    FunctionPtr f6 = new AttractiveSectorFunction(coefs, fopt);
    FunctionPtr f7 = new StepEllipsoidalFunction(coefs, fopt);
    FunctionPtr f8 = new RosenbrockFunction(coefs, fopt);
    FunctionPtr f9 = new RosenbrockRotatedFunction(coefs, fopt);
    FunctionPtr f10 = new IllEllipsoidalFunction(coefs, fopt);
    FunctionPtr f11 = new DiscusFunction(coefs, fopt);
    FunctionPtr f12 = new BentCigarFunction(coefs, fopt);
    FunctionPtr f13 = new SharpRidgeFunction(coefs, fopt);
    FunctionPtr f14 = new DifferentPowersFunction(coefs, fopt);

    OptimizerPtr optimizer;
    OptimizerContextPtr optimizerContext;
    OptimizerStatePtr optimizerState;
    SamplerPtr sampler;
    FunctionPtr f;
    
    size_t numIterations = 20;
    size_t populationSize = 100;
    size_t numBests = 30;
    f = f1;
    
    // test the function at kown global optimum
    DenseDoubleVectorPtr input = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, doubleType, 2);
    input->setValue(0, 0.0);
    input->setValue(1, 0.0);
    input->setValue(2, 0.0);
    input->setValue(3, 0.0);
    input->setValue(4, 0.0);
    double output = f->compute(context, input).toDouble();
    std::cout << output << std::endl;
    
    // Used to compare EDA with AsyncEDA with different values of inProgressEvaluations
    /*
    double start;
    double end;
    size_t nbIter = 20;
    context.progressCallback(new ProgressionState(0, nbIter, T("Iterations")));
    for (size_t i = 0; i < nbIter; i++) {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      
      // EDAOptimizer
      context.enterScope(T("eda_f10_noslowing"));
      context.resultCallback(T("inProgressEvaluations"), (int) 0);
      sampler = independentDoubleVectorSampler(5, gaussianSampler(0.0, 5.0));
      optimizer = edaOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0);
      optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 1);
      optimizerState = new SamplerBasedOptimizerState(sampler);
      start = Time::getMillisecondCounter() / 1000.0;
      optimizer->compute(context, optimizerContext, optimizerState);
      end = Time::getMillisecondCounter() / 1000.0;
      context.resultCallback(T("time"), end-start);
      context.leaveScope();
      
      size_t inProgressEvaluationsTab[] = {150,100,50,30,20,10};
      for (size_t j = 0; j < 6; j++) {
        inProgressEvaluations = inProgressEvaluationsTab[j];
        context.enterScope(T("asynceda_f10_noslowing_") + String((int)inProgressEvaluations));
        context.resultCallback(T("inProgressEvaluations"), (int) inProgressEvaluations);
        sampler = independentDoubleVectorSampler(5, gaussianSampler(0.0, 5.0));
        optimizer = asyncEDAOptimizer(numIterations, populationSize, numBests, inProgressEvaluations, StoppingCriterionPtr(), 0.0);
        optimizerContext = multiThreadedOptimizerContext(context, f, FunctionPtr(), 1);
        optimizerState = new SamplerBasedOptimizerState(sampler);
        start = Time::getMillisecondCounter() / 1000.0;
        optimizer->compute(context, optimizerContext, optimizerState);
        end = Time::getMillisecondCounter() / 1000.0;
        context.resultCallback(T("time"), end-start);
        context.leaveScope();        
      }
      context.leaveScope();
      context.progressCallback(new ProgressionState(i + 1, nbIter, T("Iterations")));
    }
    */
    
    
    // Simple example
    sampler = independentDoubleVectorSampler(5, gaussianSampler(0.0, 5.0));
    optimizer = edaOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0);
    optimizerContext = synchroneousOptimizerContext(context, f, FunctionPtr());
    optimizerState = new SamplerBasedOptimizerState(sampler);
    optimizer->compute(context, optimizerContext, optimizerState);
  
    return Variable();
  }
};
  
  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_
