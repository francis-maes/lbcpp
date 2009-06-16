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
  SearnLearner(RankerPtr initialRanker, ActionValueFunctionPtr optimalActionValues, double beta, StoppingCriterionPtr stoppingCriterion)
    : initialRanker(initialRanker), optimalActionValues(optimalActionValues), beta(beta), stoppingCriterion(stoppingCriterion)
  {
    if (!optimalActionValues)
      this->optimalActionValues = chooseActionValues();
    if (!initialRanker)
    // todo: change default ?
      this->initialRanker = logBinomialAllPairsLinearRanker(batchLearner(lbfgsOptimizer(), 50));
    if (!stoppingCriterion)
      stoppingCriterion = maxIterationsStoppingCriterion(10);
  }
  SearnLearner() : beta(0.0) {}
  
  virtual PolicyPtr getPolicy() const
    {return learnedPolicy;}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    if (progress)
      progress->progressStart("SearnLearner::trainBatch");

    learnedPolicy = PolicyPtr();
    PolicyPtr currentPolicy = greedyPolicy(optimalActionValues);
    size_t iteration = 0;
    stoppingCriterion->reset();
    while (true)
    {
      double rewardPerEpisode;
      ObjectContainerPtr classificationExamples = createCostSensitiveClassificationExamples(examples, currentPolicy, rewardPerEpisode);
      if (progress && !progress->progressStep("SearnLearner::trainBatch, reward/episode = " + lbcpp::toString(rewardPerEpisode), (double)iteration))
        return false;
      std::cout << "ITERATION " << iteration << "currentPolicy = " << currentPolicy->toString();
      if (learnedPolicy)
        std::cout << " learnedPolicy = " << learnedPolicy->toString();
      std::cout << std::endl;

      RankerPtr ranker = initialRanker->cloneAndCast<Ranker>();
      ranker->trainBatch(classificationExamples);
      PolicyPtr newPolicy = greedyPolicy(predictedActionValues(ranker));
      currentPolicy = mixturePolicy(currentPolicy, newPolicy, beta);
      learnedPolicy = learnedPolicy ? mixturePolicy(learnedPolicy, newPolicy, beta) : newPolicy;
      if (stoppingCriterion->shouldCRAlgorithmLearnerStop(learnedPolicy, examples))
        break;
      ++iteration;
    }

    if (progress)
      progress->progressEnd();
    return true;
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, initialRanker) && read(istr, optimalActionValues) && read(istr, beta) && read(istr, stoppingCriterion);}
  
  virtual void save(std::ostream& ostr) const
    {write(ostr, initialRanker); write(ostr, optimalActionValues); write(ostr, beta); write(ostr, stoppingCriterion);}

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
    PolicyStatisticsPtr statistics = new PolicyStatistics();
    policy->run(examples, statistics);
    rewardPerEpisode = statistics->getRewardPerEpisodeMean();
    return res;
  }
  
private:
  RankerPtr initialRanker;
  ActionValueFunctionPtr optimalActionValues;
  double beta;
  StoppingCriterionPtr stoppingCriterion;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_SEARN_H_
