/*-----------------------------------------.---------------------------------.
| Filename: Searn.cpp                      | Searn Learner base class        |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CRALGORITHM_LEARNER_SEARN_H_
# define LBCPP_CRALGORITHM_LEARNER_SEARN_H_

# include <lbcpp/CRAlgorithmLearner.h>
# include <lbcpp/GradientBasedLearningMachine.h>
# include <lbcpp/Optimizer.h>

namespace lbcpp
{

class SearnLearner : public CRAlgorithmLearner
{
public:
  SearnLearner(RankerPtr initialRanker, ActionValueFunctionPtr optimalActionValues, double beta, size_t numIterations)
    : initialRanker(initialRanker), optimalActionValues(optimalActionValues), beta(beta), numIterations(numIterations)
  {
    if (!optimalActionValues)
      this->optimalActionValues = chooseActionValues();
    if (!initialRanker)
    // todo: change default ?
      this->initialRanker = logBinomialAllPairsLinearRanker(batchLearner(lbfgsOptimizer(), 50));
  }
  
  virtual PolicyPtr getPolicy() const
    {return learnedPolicy;}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    if (progress)
      progress->progressStart("SearnLearner::trainBatch");

    learnedPolicy = PolicyPtr();
    PolicyPtr currentPolicy = greedyPolicy(optimalActionValues);
    for (size_t i = 0; i < numIterations; ++i)
    {
      double rewardPerEpisode;
      ObjectContainerPtr classificationExamples = createCostSensitiveClassificationExamples(examples, currentPolicy, rewardPerEpisode);
      if (progress && !progress->progressStep("SearnLearner::trainBatch, reward/episode = " + lbcpp::toString(rewardPerEpisode), (double)i, (double)numIterations))
        return false;
      std::cout << "ITERATION " << i << "currentPolicy = " << currentPolicy->toString();
      if (learnedPolicy)
        std::cout << " learnedPolicy = " << learnedPolicy->toString();
      std::cout << std::endl;

      RankerPtr ranker = initialRanker->cloneAndCast<Ranker>();
      ranker->trainBatch(classificationExamples);
      PolicyPtr newPolicy = greedyPolicy(predictedActionValues(ranker));
      currentPolicy = mixturePolicy(currentPolicy, newPolicy, beta);
      learnedPolicy = learnedPolicy ? mixturePolicy(learnedPolicy, newPolicy, beta) : newPolicy;
    }

    if (progress)
      progress->progressEnd();
    return true;
  }

protected:
  PolicyPtr learnedPolicy;
  
  struct StoreExamplesRanker : public Ranker
  {
    StoreExamplesRanker(VectorObjectContainerPtr examples)
      : examples(examples) {}
    
    virtual double predictScore(const FeatureGeneratorPtr input) const
      {return 0.0;}
    virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
      {}
    virtual void trainStochasticExample(ObjectPtr example)
      {examples->append(example);}
    virtual void trainStochasticEnd()
      {}
    virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
      {assert(false); return false;}
    virtual FeatureDictionaryPtr getInputDictionary() const
      {return FeatureDictionaryPtr();}
      
    VectorObjectContainerPtr examples;
  };

  ObjectContainerPtr createCostSensitiveClassificationExamples(ObjectContainerPtr examples, PolicyPtr explorationPolicy, double& rewardPerEpisode)
  {
    VectorObjectContainerPtr res = new VectorObjectContainer();
    PolicyPtr policy = rankingExampleCreatorPolicy(explorationPolicy, new StoreExamplesRanker(res), optimalActionValues);
    PolicyPtr p = policy->addComputeStatistics();
    p->run(examples);
    rewardPerEpisode = p->getResultWithName("rewardPerEpisode").dynamicCast<ScalarRandomVariableStatistics>()->getMean();
    return res;
  }
  
private:
  RankerPtr initialRanker;
  ActionValueFunctionPtr optimalActionValues;
  double beta;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_SEARN_H_
