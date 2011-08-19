/*-----------------------------------------.---------------------------------.
| Filename: PopulationBasedOptimizer.h     | Abstract class that contains    |
| Author  : Arnaud Schoofs                 | common methods for population   |
| Started : 19/05/2011                     | based algorithm                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

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
  PopulationBasedOptimizer(const SamplerPtr& sampler, size_t numIterations, size_t populationSize, size_t numBests, StoppingCriterionPtr stoppingCriterion, double slowingFactor = 0, bool reinjectBest = false, bool verbose = false)
    : sampler(sampler), numIterations(numIterations), populationSize(populationSize), numBests(numBests), stoppingCriterion(stoppingCriterion), slowingFactor(slowingFactor), reinjectBest(reinjectBest), verbose(verbose)
    {
      jassert(slowingFactor >= 0 && slowingFactor <= 1);
      jassert(numBests < populationSize);
    }

  virtual OptimizerStatePtr createOptimizerState(ExecutionContext& context) const
    {return new SamplerBasedOptimizerState(sampler);}
  
protected:
  friend class PopulationBasedOptimizerClass;
  
  SamplerPtr sampler;
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
  StoppingCriterionPtr stoppingCriterion;
  double slowingFactor; /**< if != 0, the distribution learned is a Mixture: with probability "slowingFactor" the old distribution and with probability "1-slowingFactor" the new one. */
  bool reinjectBest;  /**< if true the best individu is inserted in each new population. */
  bool verbose; /**< if true increase the verbosity. */

  PopulationBasedOptimizer()
    : numIterations(0), populationSize(0), numBests(0), slowingFactor(0), reinjectBest(false), verbose(false) {}

  /**
   * Creates a new individu from the distribution on OptimizerState.
   */
  Variable sampleCandidate(ExecutionContext& context, const SamplerBasedOptimizerStatePtr& state) const
    {return state->getSampler()->sample(context, context.getRandomGenerator());}
  
  /**
   * Push the results of the OptimizerState buffer in the sorted map "sortedScore"
   */
  void pushResultsSortedbyScore(ExecutionContext& context, const ContainerPtr results, const std::vector<Variable>& parameters, std::multimap<double, Variable>& sortedScores) const
  {
    const size_t n = results->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const double score = results->getElement(i).getDouble();
      sortedScores.insert(std::pair<double, Variable>(score, parameters[i]));
      if (verbose) 
      {
        context.enterScope(T("Request ") + String((int) (i+1)));
        context.resultCallback(T("requestNumber"), i+1);
        context.resultCallback(T("parameter"), parameters[i]);      
        context.leaveScope(score);
      }
    }
  }
  
  /**
   * Learn a new distribution from the results in sortedScores.
   * The OptimizerState is updated.
   */
  void learnDistribution(ExecutionContext& context, const SamplerBasedOptimizerStatePtr& state, const std::multimap<double, Variable>& sortedScores) const
  {
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

    SamplerPtr newSampler = state->getCloneOfInitialSamplerInstance();  // get instance from prototype
    newSampler->learn(context, ContainerPtr(), bestVariablesVector);
    
    if (slowingFactor > 0) 
    {
      SamplerPtr oldSampler = state->getSampler();
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
      state->setSampler(mixtureSampler(probabilities, vec));
    }
    else 
      state->setSampler(newSampler);

    context.resultCallback(T("sampler"), state->getSampler());
  }
  
  void handleResultOfIteration(ExecutionContext& context, const SamplerBasedOptimizerStatePtr& state, const FunctionPtr& validationFunction, double bestIterationScore, const Variable& bestIterationParameters) const
  {
    // update OptimizerState if necessary
    if (bestIterationScore < state->getBestScore())
    {
      state->setBestScore(bestIterationScore);
      state->setBestParameters(bestIterationParameters);
    }
    
    // give information for execution trace
    context.resultCallback(T("bestIterationParameters"), bestIterationParameters);
    context.resultCallback(T("bestIterationScore"), bestIterationScore);

    double validationScore = 0.0;
    if (validationFunction)
    {
      validationScore = validationFunction->compute(context, bestIterationParameters).toDouble();
      context.resultCallback(T("validationScore"), validationScore);
    }

    context.resultCallback(T("allTimesBestParameters"), state->getBestParameters());
    context.resultCallback(T("allTimesBestScore"), state->getBestScore());
    
    // bestIterationScore may be diffrent from optimizerState->getBestScore(),
    // this is the return value of performEDAIteration, not the best score of all time !!!
    context.leaveScope(validationFunction ? Variable(new Pair(bestIterationScore, validationScore)) : Variable(bestIterationScore)); 
  }
};
  
typedef ReferenceCountedObjectPtr<PopulationBasedOptimizer> PopulationBasedOptimizerPtr;  

}; /* namespace lbcpp */

#endif // !LBCPP_POPULATION_BASED_OPTIMIZER_H_
