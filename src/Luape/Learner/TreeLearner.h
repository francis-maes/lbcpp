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

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    objective->initialize(problem);
    conditionLearner->setObjective(objective);
    LuapeNodePtr res = makeTree(context, problem, examples, 1);
    
    size_t treeDepth = 0;
    size_t numNodes = getNumTestNodes(res, 0, treeDepth);
    context.resultCallback(T("treeDepth"), treeDepth);
    context.resultCallback(T("treeSize"), numNodes);
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

  LuapeNodePtr makeTree(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    // min examples and max depth conditions to make a leaf
    if ((examples->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth))
      return new LuapeConstantNode(objective->computeVote(examples));

    // learn condition and make a leaf if condition learning fails
    double conditionObjectiveValue;
    LuapeNodePtr conditionNode = subLearn(context, conditionLearner, LuapeNodePtr(), problem, examples, &conditionObjectiveValue);
    if (!conditionNode || conditionNode.isInstanceOf<LuapeConstantNode>())
      return new LuapeConstantNode(objective->computeVote(examples));
    conditionNode->addImportance(conditionObjectiveValue * examples->size() / problem->getTrainingCache()->getNumSamples());

    // otherwise split examples...
    LuapeSampleVectorPtr conditionValues = problem->getTrainingCache()->getSamples(context, conditionNode, examples);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    LuapeTestNode::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    if (failureExamples->size() == examples->size() || successExamples->size() == examples->size() || missingExamples->size() == examples->size())
      return new LuapeConstantNode(objective->computeVote(examples));

    // ...call recursively
    if (verbose)
      context.enterScope(conditionNode->toShortString() + T(" ") + String((int)examples->size()) + T(" -> ") + String((int)failureExamples->size()) + T("; ") + String((int)successExamples->size()) + T("; ") + String((int)missingExamples->size()));
    LuapeNodePtr failureNode = makeTree(context, problem, failureExamples, depth + 1);
    LuapeNodePtr successNode = makeTree(context, problem, successExamples, depth + 1);
    LuapeNodePtr missingNode = makeTree(context, problem, missingExamples, depth + 1);
    if (verbose)
      context.leaveScope();

    // and build a test node.
    return new LuapeTestNode(conditionNode, failureNode, successNode, missingNode);
  }

  size_t getNumTestNodes(const LuapeNodePtr& node, size_t depth, size_t& maxDepth) const
  {
    if (depth > maxDepth)
      maxDepth = depth;
    size_t res = 0;
    if (node.isInstanceOf<LuapeTestNode>())
      ++res;
    for (size_t i = 0; i < node->getNumSubNodes(); ++i)
      res += getNumTestNodes(node->getSubNode(i), depth + 1, maxDepth);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
