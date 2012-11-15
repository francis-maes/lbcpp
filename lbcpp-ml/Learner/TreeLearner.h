/*-----------------------------------------.---------------------------------.
| Filename: TreeLearner.h                  | Tree Learner                    |
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LEARNER_TREE_H_
# define LBCPP_ML_LEARNER_TREE_H_

# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/SplittingCriterion.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

class TreeLearner : public Solver
{
public:
  TreeLearner(SplittingCriterionPtr splittingCriterion, SolverPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth)
    : splittingCriterion(splittingCriterion), conditionLearner(conditionLearner), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth) {}
  TreeLearner() {}

  virtual void runSolver(ExecutionContext& context)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();

    ExpressionPtr res = makeTreeScope(context, objective, objective->getIndices(), 1);
    if (verbosity >= verbosityProgressAndResult)
    {
      size_t treeDepth = 0;
      ScalarVariableStatisticsPtr nodeSizeStats = new ScalarVariableStatistics(T("nodeSize"));
      size_t numNodes = getNumTestNodes(res, 0, treeDepth, nodeSizeStats);
      context.resultCallback(T("treeDepth"), treeDepth);
      context.resultCallback(T("treeSize"), numNodes);
      context.resultCallback(T("conditionSize"), nodeSizeStats);
      context.resultCallback(T("meanConditionSize"), nodeSizeStats->getMean());
      context.informationCallback(T("Tree depth = ") + string((int)treeDepth) + T(" size = ") + string((int)numNodes));
    }
    evaluate(context, res);
  }

protected:
  friend class TreeLearnerClass;

  SplittingCriterionPtr splittingCriterion;
  SolverPtr conditionLearner;
  size_t minExamplesToSplit;
  size_t maxDepth;

  bool isConstant(const VectorPtr& data, const IndexSetPtr& indices) const
  {
    if (indices->size() <= 1)
      return true;
    IndexSet::const_iterator it = indices->begin();
    ObjectPtr value = data->getElement(*it);
    for (++it; it != indices->end(); ++it)
      if (!Object::equals(value, data->getElement(*it)))
        return false;
    return true;
  }
  
  ExpressionPtr makeTreeScope(ExecutionContext& context, const SupervisedLearningObjectivePtr& objective, const IndexSetPtr& indices, size_t depth)
  {
    if (verbosity >= verbosityDetailed)
      context.enterScope(T("Make tree with ") + string((int)indices->size()) + " examples");
    ExpressionPtr res = makeTree(context, objective, indices, depth);
    if (verbosity >= verbosityDetailed)
      context.leaveScope();
    return res;
  }

  ExpressionPtr makeTree(ExecutionContext& context, const SupervisedLearningObjectivePtr& objective, const IndexSetPtr& indices, size_t depth)
  {
    // min examples and max depth conditions to make a leaf
    if ((indices->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth) || isConstant(objective->getSupervisions(), indices))
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    // launch condition learner
    splittingCriterion->configure(objective->getData(), objective->getSupervision(), DenseDoubleVectorPtr(), indices);
    ProblemPtr conditionProblem = new Problem(problem->getDomain(), splittingCriterion);
    ExpressionPtr conditionNode;
    FitnessPtr conditionFitness;
    conditionLearner->solve(context, conditionProblem, storeBestSolverCallback(*(ObjectPtr* )&conditionNode, conditionFitness));

    // check condition and update importance values
    if (!conditionNode || conditionNode.isInstanceOf<ConstantExpression>())
      return new ConstantExpression(splittingCriterion->computeVote(indices));
    conditionNode->addImportance(conditionFitness->getValue(0) * indices->size() / objective->getNumSamples());
    if (verbosity >= verbosityDetailed)
      context.informationCallback(conditionNode->toShortString() + T(" [") + string(conditionNode->getSubNode(0)->getImportance()) + T("]"));

    // otherwise split examples...
    DataVectorPtr conditionValues = conditionNode->compute(context, objective->getData(), indices);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    TestExpression::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    if (failureExamples->size() == indices->size() || successExamples->size() == indices->size() || missingExamples->size() == indices->size())
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    // ...call recursively
    if (verbosity >= verbosityDetailed)
      context.enterScope(conditionNode->toShortString());
    ExpressionPtr failureNode = makeTreeScope(context, objective, failureExamples, depth + 1);
    ExpressionPtr successNode = makeTreeScope(context, objective, successExamples, depth + 1);
    ExpressionPtr missingNode = makeTreeScope(context, objective, missingExamples, depth + 1);
    if (verbosity >= verbosityDetailed)
      context.leaveScope();

    // and build a test node.
    return new TestExpression(conditionNode, failureNode, successNode, missingNode);
  }

  size_t getNumTestNodes(const ExpressionPtr& node, size_t depth, size_t& maxDepth, ScalarVariableStatisticsPtr nodeSizeStats) const
  {
    if (depth > maxDepth)
      maxDepth = depth;
    size_t res = 0;
    TestExpressionPtr testNode = node.dynamicCast<TestExpression>();
    if (testNode)
    {
      nodeSizeStats->push(testNode->getCondition()->getTreeSize());
      ++res;
      res += getNumTestNodes(testNode->getFailure(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getSuccess(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getMissing(), depth + 1, maxDepth, nodeSizeStats);
    }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LEARNER_TREE_H_
