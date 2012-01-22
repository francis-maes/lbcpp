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

  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!IterativeLearner::initialize(context, node, problem, examples))
      return false;
    weakLearner->setObjective(objective);
    return true;
  }

  virtual void contributionAdded(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& contribution) {}

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
      if (contribution)
      {
        node.staticCast<LuapeSequenceNode>()->pushNode(context, contribution, problem->getSamplesCaches());
        contributionAdded(context, problem, contribution);
      }
    }

    // evaluate
    if (verbose)
      evaluatePredictions(context, problem, trainingScore, validationScore);

    // trainingCache->checkCacheIsCorrect(context, function->getRootNode());
    if (verbose)
      context.resultCallback(T("contribution"), verbose ? Variable(contribution) : Variable(contribution->toShortString()));
    return true;
  }

  const LuapeLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    IterativeLearner::clone(context, target);
    if (weakLearner)
      target.staticCast<BoostingLearner>()->weakLearner = weakLearner->cloneAndCast<LuapeLearner>(context);
  }

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
    weakNode->addImportance(weakObjective);
    LuapeNodePtr contribution = turnWeakNodeIntoContribution(context, weakNode, problem, examples, weakObjective);
    if (depth == treeDepth || weakNode.isInstanceOf<LuapeConstantNode>())
      return contribution;


    LuapeTestNodePtr testNode = contribution.dynamicCast<LuapeTestNode>();
    if (!testNode)
      testNode = new LuapeTestNode(contribution.staticCast<LuapeFunctionNode>()->getSubNode(0), contribution->getType()); // extract weak predictor from vote

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

  virtual LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double weakObjective) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
