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
# include <lbcpp/Data/IndexSet.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

class BoostingLearner : public IterativeLearner
{
public:
  BoostingLearner(LearningObjectivePtr objective, LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : IterativeLearner(objective, maxIterations), weakLearner(weakLearner), treeDepth(treeDepth) {}
  BoostingLearner() : treeDepth(0) {}

  virtual bool computeVotes(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeSampleVectorPtr& weakPredictions, Variable& failureVote, Variable& successVote, Variable& missingVote) const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!IterativeLearner::initialize(context, node, problem, examples))
      return false;
    weakLearner->setObjective(objective);
    return true;
  }

  virtual bool doLearningIteration(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    LuapeNodePtr contribution;
   
    // do weak learning
    {
      TimedScope _(context, "weak learning", verbose);
      contribution = learnContribution(context, problem, examples, 1);
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

  const LuapeLearnerPtr& getWeakLearner() const
    {return weakLearner;}

protected:
  friend class BoostingLearnerClass;
  
  LuapeLearnerPtr weakLearner;
  size_t treeDepth;

  LuapeNodePtr learnContribution(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    if (examples->size() < 2)
      return LuapeNodePtr();
    double weakObjective;
    LuapeNodePtr weakNode = subLearn(context, weakLearner, LuapeNodePtr(), problem, examples, &weakObjective);
    if (!weakNode)
    {
      if (depth == 1)
        context.errorCallback(T("Failed to find a weak learner"));
      return LuapeNodePtr();
    }
    context.resultCallback(T("edge"), weakObjective);
    LuapeNodePtr res = turnWeakNodeIntoContribution(context, weakNode, problem, examples, weakObjective);
    if (depth == treeDepth || weakNode.isInstanceOf<LuapeConstantNode>())
      return res;

    LuapeTestNodePtr testNode = res.staticCast<LuapeTestNode>();
    LuapeSampleVectorPtr testValues = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    testNode->dispatchIndices(testValues, failureExamples, successExamples, missingExamples);

    LuapeNodePtr failureNode = learnContribution(context, problem, failureExamples, depth + 1);
    if (failureNode)
      testNode->setFailure(failureNode);
    LuapeNodePtr successNode = learnContribution(context, problem, successExamples, depth + 1);
    if (successNode)
      testNode->setSuccess(successNode);
    LuapeNodePtr missingNode = learnContribution(context, problem, missingExamples, depth + 1);
    if (missingNode)
      testNode->setMissing(missingNode);
    return testNode;
  }

  LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double weakObjective) const
  {
    jassert(weakNode);
    LuapeSampleVectorPtr weakPredictions = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    Variable failureVote, successVote, missingVote;
    if (!computeVotes(context, problem, weakPredictions, failureVote, successVote, missingVote))
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
      res = new LuapeTestNode(weakNode, new LuapeConstantNode(failureVote), new LuapeConstantNode(successVote), new LuapeConstantNode(missingVote));
    if (verbose)
      context.informationCallback(res->toShortString());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
