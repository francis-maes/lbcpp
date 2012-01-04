/*-----------------------------------------.---------------------------------.
| Filename: BoostingLearner.h              | Base class for boosting         |
| Author  : Francis Maes                   |   learners                      |
| Started : 04/01/2012 13:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_BOOSTING_H_
# define LBCPP_LUAPE_LEARNER_BOOSTING_H_

# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/WeakLearner.h>
# include <lbcpp/Data/IndexSet.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

class BoostingLearner : public IterativeLearner
{
public:
  BoostingLearner(WeakLearnerPtr weakLearner, size_t maxIterations)
    : IterativeLearner(maxIterations), weakLearner(weakLearner) {}
  BoostingLearner() {}

  virtual WeakLearnerObjectivePtr createWeakObjective(const LuapeInferencePtr& problem) const = 0;
  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const = 0;

  const WeakLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  virtual bool doLearningIteration(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    LuapeNodePtr contribution;
   
    // do weak learning
    {
      TimedScope _(context, "weak learning", verbose);
      weakLearner->setWeakObjective(createWeakObjective(problem));
      LuapeNodePtr weakNode = weakLearner->learn(context, node, problem, examples);
      double weakObjective = weakLearner->getBestWeakObjectiveValue();
      if (!weakNode || weakObjective == -DBL_MAX)
      {
        context.errorCallback(T("Failed to find a weak learner"));
        return false;
      }
      contribution = turnWeakNodeIntoContribution(context, weakNode, problem, examples, weakObjective);
      context.resultCallback(T("edge"), weakObjective);
    }

    // add into node and caches
    {
      TimedScope _(context, "add into node", verbose);
      node.staticCast<LuapeSequenceNode>()->pushNode(context, contribution, problem->getSamplesCaches());
    }

    // evaluate
    evaluatePredictions(context, problem, trainingScore, validationScore);

    // trainingCache->checkCacheIsCorrect(context, function->getRootNode());
    if (verbose)
      context.resultCallback(T("contribution"), verbose ? Variable(contribution) : Variable(contribution->toShortString()));
    return true;
  }

protected:
  friend class BoostingLearnerClass;
  
  WeakLearnerPtr weakLearner;

  LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double weakObjective) const
  {
    jassert(weakNode);
    Variable successVote, failureVote, missingVote;
    if (!computeVotes(context, weakNode, problem, examples, successVote, failureVote, missingVote))
      return LuapeNodePtr();

    weakNode->addImportance(weakObjective);

    LuapeNodePtr res;
    jassert(weakNode->getType() == booleanType || weakNode->getType() == probabilityType);
    if (weakNode.isInstanceOf<LuapeConstantNode>())
    {
      LuapeConstantNodePtr constantNode = weakNode.staticCast<LuapeConstantNode>();
      Variable constantValue = constantNode->getValue();
      jassert(constantValue.isBoolean());
      if (constantValue.isMissingValue())
        res = new LuapeConstantNode(missingVote);
      else if (constantValue.getBoolean())
        res = new LuapeConstantNode(successVote);
      else
        res = new LuapeConstantNode(failureVote);
    }
    else
      res = new LuapeTestNode(weakNode, new LuapeConstantNode(successVote), new LuapeConstantNode(failureVote), new LuapeConstantNode(missingVote));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
