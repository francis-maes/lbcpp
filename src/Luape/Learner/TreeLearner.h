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
    : objective(objective), conditionLearner(conditionLearner), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth) {}
  TreeLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    objective->initialize(problem);
    conditionLearner->setObjective(objective);
    return makeTree(context, problem, examples, 1);
  }

protected:
  friend class TreeLearnerClass;

  LuapeLearnerPtr conditionLearner;
  LearningObjectivePtr objective;
  size_t minExamplesToSplit;
  size_t maxDepth;

  LuapeNodePtr makeTree(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    // min examples and max depth conditions to make a leaf
    if ((examples->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth))
      return new LuapeConstantNode(objective->computeVote(examples));

    // learn condition and make a leaf if condition learning fails
    LuapeNodePtr conditionNode = subLearn(context, conditionLearner, LuapeNodePtr(), problem, examples);
    if (!conditionNode)
      return new LuapeConstantNode(objective->computeVote(examples));

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
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
