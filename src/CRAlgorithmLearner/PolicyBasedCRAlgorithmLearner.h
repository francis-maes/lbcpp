/*-----------------------------------------.---------------------------------.
| Filename: PolicyBasedCRAlgorithmLearner.h| Base class for CRAlgorithm      |
| Author  : Francis Maes                   |   learners based on             |
| Started : 16/06/2009 15:38               |   Policy linear execution       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CRALGORITHM_LEARNER_POLICY_BASED_H_
# define LBCPP_CRALGORITHM_LEARNER_POLICY_BASED_H_

# include <lbcpp/CRAlgorithmLearner.h>

namespace lbcpp
{

class PolicyBasedCRAlgorithmLearner : public CRAlgorithmLearner
{
public:
  PolicyBasedCRAlgorithmLearner(ExplorationType exploration, IterationFunctionPtr explorationParameter, StoppingCriterionPtr stoppingCriterion)
    : exploration(exploration), explorationParameter(explorationParameter), stoppingCriterion(stoppingCriterion)
  {
    if (!stoppingCriterion)
      this->stoppingCriterion = maxIterationsStoppingCriterion(100);
  }
  
  PolicyBasedCRAlgorithmLearner() : exploration(random) {}
        
  virtual PolicyPtr getPolicy() const
    {return learnedPolicy;}

  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary)
  {
    if (!learnedPolicy)
      initialize();
  }
  
  virtual void trainStochasticExample(ObjectPtr example)
  {
    CRAlgorithmPtr crAlgorithm = example.dynamicCast<CRAlgorithm>();
    jassert(crAlgorithm);
    crAlgorithm->run(learnerPolicy);
  }
  
  virtual void trainStochasticEnd()
    {}

  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    if (progress)
      progress->progressStart("PolicyBasedCRAlgorithmLearner::trainBatch");
    initialize();
    jassert(stoppingCriterion);
    stoppingCriterion->reset();
    size_t iteration = 0;
    while (true)
    {
      if (progress && !progress->progressStep("PolicyBasedCRAlgorithmLearner::trainBatch", iteration))
        return false;
      PolicyStatisticsPtr statistics = new PolicyStatistics();
      learnerPolicy->run(examples, statistics);
      std::cout << statistics->toString() << std::endl;
      if (stoppingCriterion->shouldCRAlgorithmLearnerStop(learnedPolicy, examples))
        break;
      ++iteration;
    }
    if (progress)
      progress->progressEnd();
    return true;
  }

protected:
  virtual ActionValueFunctionPtr createLearnedActionValues() const = 0;
  virtual PolicyPtr createLearnerPolicy(PolicyPtr explorationPolicy) const = 0;

protected:
  ExplorationType exploration;
  IterationFunctionPtr explorationParameter;
  StoppingCriterionPtr stoppingCriterion;

  ActionValueFunctionPtr learnedActionValues;
  PolicyPtr learnedPolicy;
  PolicyPtr explorationPolicy;
  PolicyPtr learnerPolicy;
  
  void initialize()
  {
    learnedActionValues = createLearnedActionValues();
    learnedPolicy = greedyPolicy(learnedActionValues);
    explorationPolicy = createExplorationPolicy(learnedActionValues);
    learnerPolicy = createLearnerPolicy(explorationPolicy);
  }

  PolicyPtr createExplorationPolicy(ActionValueFunctionPtr learnedActionValues) const
  {
    switch (exploration)
    {
    case optimal:
      return greedyPolicy(chooseActionValues());
    case optimalEpsilonGreedy:
      return epsilonGreedyPolicy(greedyPolicy(chooseActionValues()), explorationParameter);
    case optimalGibbsGreedy:
      return gibbsGreedyPolicy(chooseActionValues(), explorationParameter);

    case optimalToPredicted:
      return mixturePolicy(greedyPolicy(chooseActionValues()), greedyPolicy(learnedActionValues), explorationParameter);

    case predicted:
      return greedyPolicy(learnedActionValues);
    case predictedEpsilonGreedy:
      return epsilonGreedyPolicy(greedyPolicy(learnedActionValues), explorationParameter);
    case predictedGibbsGreedy:
      return gibbsGreedyPolicy(learnedActionValues, explorationParameter);
    case predictedProbabilistic:
      return stochasticPolicy(learnedActionValues);

    case random:
      return randomPolicy();

    default:
      jassert(false);
      return PolicyPtr();
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_POLICY_BASED_H_
