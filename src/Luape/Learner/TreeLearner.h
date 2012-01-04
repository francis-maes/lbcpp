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
  TreeLearner(WeakLearnerPtr conditionLearner, WeakLearnerObjectivePtr objective, size_t minExamplesToSplit, size_t maxDepth)
    : conditionLearner(conditionLearner), objective(objective), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth) {}
  TreeLearner() {}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
    {return makeTree(context, problem, examples, 1);}

protected:
  friend class TreeLearnerClass;

  WeakLearnerPtr conditionLearner;
  WeakLearnerObjectivePtr objective;
  size_t minExamplesToSplit;
  size_t maxDepth;

  LuapeNodePtr makeTree(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples, size_t depth)
  {
    // min examples and max depth conditions to make a leaf
    if ((examples->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth))
      return new LuapeConstantNode(objective->computeVote(examples));

    // learn condition and make a leaf if condition learning fails
    conditionLearner->setWeakObjective(objective);
    LuapeNodePtr conditionNode = conditionLearner->learn(context, LuapeNodePtr(), problem, examples);
    if (!conditionNode)
      return new LuapeConstantNode(objective->computeVote(examples));

    // otherwise split examples...
    LuapeSampleVectorPtr conditionValues = problem->getTrainingCache()->getSamples(context, conditionNode, examples);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    LuapeTestNode::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    // ...call recursively
    LuapeNodePtr failureNode = makeTree(context, problem, failureExamples, depth + 1);
    LuapeNodePtr successNode = makeTree(context, problem, failureExamples, depth + 1);
    LuapeNodePtr missingNode = makeTree(context, problem, failureExamples, depth + 1);

    // and build a test node.
    return new LuapeTestNode(conditionNode, failureNode, successNode, missingNode);
  }
};

class ClassificationTreeLearner : public TreeLearner
{
public:
  ClassificationTreeLearner(WeakLearnerPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth)
    : TreeLearner(conditionLearner, WeakLearnerObjectivePtr(), minExamplesToSplit, maxDepth)
  {
  }

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    ClassPtr dvClass = problem.staticCast<LuapeClassifier>()->getDoubleVectorClass();
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions(); // FIXME: must be converted
    DenseDoubleVectorPtr weights = new DenseDoubleVector(problem->getTrainingCache()->getNumSamples(), 1.0); // FIXME: must be initialized
    objective = new AdaBoostMHWeakObjective(dvClass, supervisions, weights);
    return TreeLearner::learn(context, node, problem, examples);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_ENSEMBLE_H_
