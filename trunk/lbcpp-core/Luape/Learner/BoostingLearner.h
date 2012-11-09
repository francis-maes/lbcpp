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
  BoostingLearner(SplitObjectivePtr objective, LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth)
    : IterativeLearner(objective, maxIterations), weakLearner(weakLearner), treeDepth(treeDepth) {}
  BoostingLearner() : treeDepth(0) {}

  virtual bool initialize(ExecutionContext& context, const ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    if (!IterativeLearner::initialize(context, node, problem, examples))
      return false;
    weakLearner->setObjective(objective);
    return true;
  }

  virtual void contributionAdded(ExecutionContext& context, const ExpressionDomainPtr& problem, const ExpressionPtr& contribution) {}

  virtual bool doLearningIteration(ExecutionContext& context, ExpressionPtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    ExpressionPtr contribution;
   
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
        node.staticCast<SequenceExpression>()->pushNode(context, contribution, problem->getSamplesCaches());
        contributionAdded(context, problem, contribution);
      }
    }

    // evaluate
    if (verbose)
      evaluatePredictions(context, problem, trainingScore, validationScore);

    //problem->getTrainingCache()->checkCacheIsCorrect(context, problem->getRootNode(), true);
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

  ExpressionPtr learnContribution(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    if (examples->size() < 2)
      return ExpressionPtr();
    double weakObjective;
    ExpressionPtr weakNode = subLearn(context, weakLearner, ExpressionPtr(), problem, examples, &weakObjective);
    if (!weakNode)
    {
      if (depth == 1)
        context.errorCallback(T("Failed to find a weak learner"));
      return ExpressionPtr();
    }
    context.resultCallback(T("edge"), weakObjective);
    weakNode->addImportance(weakObjective);
    ExpressionPtr contribution = turnWeakNodeIntoContribution(context, weakNode, problem, examples, weakObjective);
    if (depth == treeDepth || weakNode.isInstanceOf<ConstantExpression>())
      return contribution;


    TestExpressionPtr testNode = contribution.dynamicCast<TestExpression>();
    if (!testNode)
      testNode = new TestExpression(contribution.staticCast<FunctionExpression>()->getSubNode(0), contribution->getType()); // extract weak predictor from vote

    LuapeSampleVectorPtr testValues = problem->getTrainingCache()->getSamples(context, weakNode, examples);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    testNode->dispatchIndices(testValues, failureExamples, successExamples, missingExamples);

    ExpressionPtr failureNode = learnContribution(context, problem, failureExamples, depth + 1);
    if (failureNode)
      testNode->setFailure(failureNode);
    ExpressionPtr successNode = learnContribution(context, problem, successExamples, depth + 1);
    if (successNode)
      testNode->setSuccess(successNode);
    ExpressionPtr missingNode = learnContribution(context, problem, missingExamples, depth + 1);
    if (missingNode)
      testNode->setMissing(missingNode);
    return testNode;
  }

  virtual ExpressionPtr turnWeakNodeIntoContribution(ExecutionContext& context, const ExpressionPtr& weakNode, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double weakObjective) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
