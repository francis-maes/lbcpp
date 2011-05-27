/*
 *  PopulationBasedOptimizer.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 19/05/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

// TODO arnaud : header

// TODO arnaud : remove distribution when useless

#ifndef LBCPP_POPULATION_BASED_OPTIMIZER_H_
# define LBCPP_POPULATION_BASED_OPTIMIZER_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{
  
class PopulationBasedOptimizer : public Optimizer
{
public:
  PopulationBasedOptimizer(size_t numIterations, size_t populationSize, size_t numBests, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : numIterations(numIterations), populationSize(populationSize), numBests(numBests), slowingFactor(slowingFactor), reinjectBest(reinjectBest), verbose(verbose), random(new RandomGenerator()) // FIXME : random generator use less 
    {
      jassert(slowingFactor >= 0 && slowingFactor <= 1);
      jassert(numBests < populationSize);
    }
  
  PopulationBasedOptimizer()
    : numIterations(0), populationSize(0), numBests(0), slowingFactor(0), reinjectBest(false), verbose(false), random(new RandomGenerator()) {} // FIXME : random generator use less
  
protected:
  friend class PopulationBasedOptimizerClass;
  
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
  double slowingFactor;
  bool reinjectBest;
  bool verbose;
  RandomGeneratorPtr random;  // FIXME : useless
  
  Variable sampleCandidate(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const RandomGeneratorPtr& random) const
  {
    // TODO: replace by optimizerState.staticCast<SamplerBasedOptimizerState>()->getSampler()->sample(context, random);
    SamplerBasedOptimizerStatePtr samplerBasedState = optimizerState.dynamicCast<SamplerBasedOptimizerState>();
    if (samplerBasedState)
      return samplerBasedState->getSampler()->sample(context, context.getRandomGenerator());  // WARNING : uses new random generator method !!!
    
    jassert(false);
    return Variable();
  }
  
  void pushResultsSortedbyScore(ExecutionContext& context, const OptimizerStatePtr& optimizerState, std::multimap<double, Variable>& sortedScores) const
  {
    ScopedLock _(optimizerState->getLock());
    // WARNING : there may be more than populationSize results in ProcessedRequests !!! (but not less)
    jassert(optimizerState->getProcessedRequests().size() >= populationSize);
    std::vector< std::pair<double, Variable> >::const_iterator it = optimizerState->getProcessedRequests().begin();
    for (size_t i = 0; i < populationSize && it != optimizerState->getProcessedRequests().end(); ++i, ++it)
    {
      sortedScores.insert(*it);
      if (verbose) 
      {
        context.enterScope(T("Request ") + String((int) (i+1)));
        context.resultCallback(T("requestNumber"), i+1);
        context.resultCallback(T("parameter"), it->second);      
        context.leaveScope(it->first);
      }
    }
    optimizerState->flushFirstProcessedRequests(populationSize);  // TODO arnaud : maybe do that after building new distri      
  }
  
  void learnDistribution(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const std::multimap<double, Variable>& sortedScores) const
  {          
    SamplerBasedOptimizerStatePtr samplerBasedState = optimizerState.staticCast<SamplerBasedOptimizerState>();
    
    std::map<Variable, ScalarVariableStatistics> bestVariables;
    std::multimap<double, Variable>::const_iterator it;
    for (it = sortedScores.begin(); bestVariables.size() < numBests && it != sortedScores.end(); ++it)
      bestVariables[it->second].push(it->first);

    VectorPtr bestVariablesVector = vector(sortedScores.begin()->second.getType(), bestVariables.size());
    size_t i = 0;
    for (std::map<Variable, ScalarVariableStatistics>::const_iterator it = bestVariables.begin(); it != bestVariables.end(); ++it)
    {
      if (verbose && i < 10)
        context.informationCallback(it->first.toShortString() + T(": ") + it->second.toShortString());
      bestVariablesVector->setElement(i, it->first);
      ++i;
    }
    if (verbose)
      context.informationCallback(String((int)sortedScores.size()) + T(" scores, ") + String(juce::jmin((int)numBests, (int)sortedScores.size())) + T(" bests, ")
        + String((int)bestVariables.size()) + T(" unique bests"));

    SamplerPtr newSampler = samplerBasedState->getCloneOfInitialSamplerInstance();
    newSampler->learn(context, ContainerPtr(), bestVariablesVector);
    
    if (slowingFactor > 0) 
    {
      SamplerPtr oldSampler = samplerBasedState->getSampler();
      // new sampler is a mixture sampler : 
      // oldSampler with proba = slowingFactor
      // newSampler with proba = 1-slowingFactor
      DenseDoubleVectorPtr probabilities = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, probabilityType, 2);
      probabilities->setValue(0, slowingFactor);
      probabilities->setValue(1, 1-slowingFactor);
      std::vector<SamplerPtr> vec;
      vec.reserve(2);
      vec.push_back(oldSampler);
      vec.push_back(newSampler);
      samplerBasedState->setSampler(mixtureSampler(probabilities, vec));
    }
    else 
      samplerBasedState->setSampler(newSampler);

    context.resultCallback(T("sampler"), samplerBasedState->getSampler());
  }
  
  void handleResultOfIteration(ExecutionContext& context, const OptimizerStatePtr& optimizerState, const OptimizerContextPtr& optimizerContext, double bestIterationScore, const Variable& bestIterationParameters) const
  {
    // update OptimizerState if necessary
    {
      if (bestIterationScore < optimizerState->getBestScore())
      {
        ScopedLock _(optimizerState->getLock());
        optimizerState->setBestScore(bestIterationScore);
        optimizerState->setBestVariable(bestIterationParameters);
      }
    }
    
    context.resultCallback(T("bestIterationParameters"), bestIterationParameters);
    context.resultCallback(T("bestIterationScore"), bestIterationScore);
    
    FunctionPtr validationFunction = optimizerContext->getValidationFunction();
    double validationScore = 0.0;
    if (validationFunction)
    {
      validationScore = validationFunction->compute(context, bestIterationParameters).toDouble(); // TODO francis : there was an incoherence between the args here and the leaveScope
      context.resultCallback(T("validationScore"), validationScore);
    }
    
    context.resultCallback(T("allTimesBestParameters"), optimizerState->getBestVariable());
    context.resultCallback(T("allTimesBestScore"), optimizerState->getBestScore());
    
    // bestIterationScore may be diffrent from optimizerState->getBestScore(),
    // this is the return value of performEDAIteration, not the best score of all time !!!
    context.leaveScope(validationFunction ? Variable(new Pair(bestIterationScore, validationScore)) : Variable(bestIterationScore)); 
        
    optimizerState->autoSaveToFile(context);  // TODO arnaud : flag to force to save at end of iteration ?
  }
};
  
typedef ReferenceCountedObjectPtr<PopulationBasedOptimizer> PopulationBasedOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_POPULATION_BASED_OPTIMIZER_H_
