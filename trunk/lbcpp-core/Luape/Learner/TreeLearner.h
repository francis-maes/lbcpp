/*-----------------------------------------.---------------------------------.
| Filename: TreeLearner.h                  | Decision Tree Learner           |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2012 18:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_TREE_H_
# define LBCPP_LUAPE_LEARNER_TREE_H_

# include <lbcpp/Luape/LuapeLearner.h>

namespace lbcpp
{

class TreeLearner : public LuapeLearner
{
public:
  TreeLearner(LearningObjectivePtr objective, LuapeLearnerPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth)
    : LuapeLearner(objective), conditionLearner(conditionLearner), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth) {}
  TreeLearner() {}

  virtual ExpressionPtr learn(ExecutionContext& context, const ExpressionPtr& node, const ExpressionDomainPtr& problem, const IndexSetPtr& examples)
  {
    objective->initialize(problem);
    conditionLearner->setObjective(objective);
    ExpressionPtr res = makeTree(context, problem, examples, 1);
    
    size_t treeDepth = 0;
    ScalarVariableStatisticsPtr nodeSizeStats = new ScalarVariableStatistics(T("nodeSize"));
    size_t numNodes = getNumTestNodes(res, 0, treeDepth, nodeSizeStats);
    context.resultCallback(T("treeDepth"), treeDepth);
    context.resultCallback(T("treeSize"), numNodes);
    context.resultCallback(T("conditionSize"), nodeSizeStats);
    context.resultCallback(T("meanConditionSize"), nodeSizeStats->getMean());
    context.resultCallback(T("TMP"), res);
    if (verbose)
      context.informationCallback(T("Tree depth = ") + String((int)treeDepth) + T(" size = ") + String((int)numNodes));

    bestObjectiveValue = 0.0;
    return res;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    LuapeLearner::clone(context, target);
    if (conditionLearner)
      target.staticCast<TreeLearner>()->conditionLearner = conditionLearner->cloneAndCast<LuapeLearner>(context);
  }

protected:
  friend class TreeLearnerClass;

  LuapeLearnerPtr conditionLearner;
  size_t minExamplesToSplit;
  size_t maxDepth;

  bool isSupervisionConstant(const ExpressionDomainPtr& problem, const IndexSetPtr& examples) const
  {
    if (examples->size() <= 1)
      return true;
    VectorPtr supervisions = problem->getTrainingSupervisions();
    IndexSet::const_iterator it = examples->begin();
    Variable supervision = supervisions->getElement(*it);
    for (++it; it != examples->end(); ++it)
      if (supervisions->getElement(*it) != supervision)
        return false;
    return true;
  }

  ExpressionPtr makeTree(ExecutionContext& context, const ExpressionDomainPtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    const ExpressionUniversePtr& universe = problem->getUniverse();

    // min examples and max depth conditions to make a leaf
    if ((examples->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth) || isSupervisionConstant(problem, examples))
      return universe->makeConstantNode(objective->computeVote(examples));

    // learn condition and make a leaf if condition learning fails
    double conditionObjectiveValue;
    ExpressionPtr conditionNode = subLearn(context, conditionLearner, ExpressionPtr(), problem, examples, &conditionObjectiveValue);
    if (!conditionNode || conditionNode.isInstanceOf<ConstantExpression>())
      return universe->makeConstantNode(objective->computeVote(examples));
    conditionNode->addImportance(conditionObjectiveValue * examples->size() / problem->getTrainingCache()->getNumSamples());
//    context.informationCallback(conditionNode->toShortString() + T(" [") + String(conditionNode->getSubNode(0)->getImportance()) + T("]"));

    // otherwise split examples...
    LuapeSampleVectorPtr conditionValues = problem->getTrainingCache()->getSamples(context, conditionNode, examples);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    TestExpression::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    if (failureExamples->size() == examples->size() || successExamples->size() == examples->size() || missingExamples->size() == examples->size())
      return universe->makeConstantNode(objective->computeVote(examples));

    // ...call recursively
    if (verbose)
      context.enterScope(conditionNode->toShortString() + T(" ") + String((int)examples->size()) + T(" -> ") + String((int)failureExamples->size()) + T("; ") + String((int)successExamples->size()) + T("; ") + String((int)missingExamples->size()));
    ExpressionPtr failureNode = makeTree(context, problem, failureExamples, depth + 1);
    ExpressionPtr successNode = makeTree(context, problem, successExamples, depth + 1);
    ExpressionPtr missingNode = makeTree(context, problem, missingExamples, depth + 1);
    if (verbose)
      context.leaveScope();

    // and build a test node.
    return new TestExpression(conditionNode, failureNode, successNode, missingNode);
  }

  size_t getNodeSize(const ExpressionPtr& node) const
  {
    size_t res = 1;
    for (size_t i = 0; i < node->getNumSubNodes(); ++i)
      res += getNodeSize(node->getSubNode(i));
    return res;
  }

  size_t getNumTestNodes(const ExpressionPtr& node, size_t depth, size_t& maxDepth, ScalarVariableStatisticsPtr nodeSizeStats) const
  {
    if (depth > maxDepth)
      maxDepth = depth;
    size_t res = 0;
    TestExpressionPtr testNode = node.dynamicCast<TestExpression>();
    if (testNode)
    {
      nodeSizeStats->push(getNodeSize(testNode->getCondition()));
      ++res;
      res += getNumTestNodes(testNode->getFailure(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getSuccess(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getMissing(), depth + 1, maxDepth, nodeSizeStats);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
