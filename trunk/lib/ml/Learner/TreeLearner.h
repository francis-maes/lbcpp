/*-----------------------------------------.---------------------------------.
| Filename: TreeLearner.h                  | Tree Learner                    |
| Author  : Francis Maes                   |                                 |
| Started : 15/11/2012 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_LEARNER_TREE_H_
# define ML_LEARNER_TREE_H_

# include <ml/RandomVariable.h>
# include <ml/Expression.h>
# include <ml/SplittingCriterion.h>
# include <ml/Solver.h>

namespace lbcpp
{

class TreeLearner : public Solver
{
public:
  TreeLearner(SplittingCriterionPtr splittingCriterion, SolverPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth)
    : splittingCriterion(splittingCriterion), conditionLearner(conditionLearner), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth), isInBatch(false) {}
  TreeLearner() : isInBatch(false) {}

  virtual void startBatch(ExecutionContext& context)
  {
    isInBatch = true;
    conditionLearner->startBatch(context);
  }
    
  virtual void stopBatch(ExecutionContext& context)
  {
    conditionLearner->stopBatch(context);
    isInBatch = false;
  }

  virtual void runSolver(ExecutionContext& context)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();

    if (!isInBatch)
      conditionLearner->startBatch(context);
    ExpressionPtr res = makeTreeScope(context, objective, objective->getIndices(), 1);
    if (!isInBatch)
      conditionLearner->stopBatch(context);
    
    if (verbosity >= verbosityProgressAndResult)
    {
      size_t treeDepth = 0;
      ScalarVariableStatisticsPtr nodeSizeStats = new ScalarVariableStatistics(T("nodeSize"));
      size_t numNodes = getNumTestNodes(res, 0, treeDepth, nodeSizeStats);
      context.resultCallback(T("treeDepth"), treeDepth);
      context.resultCallback(T("treeSize"), numNodes);
      context.resultCallback(T("conditionSize"), nodeSizeStats);
      context.resultCallback(T("meanConditionSize"), nodeSizeStats->getMean());
      //context.informationCallback(T("Tree depth = ") + string((int)treeDepth) + T(" size = ") + string((int)numNodes));
    }
    evaluate(context, res);
  }

protected:
  friend class TreeLearnerClass;

  SplittingCriterionPtr splittingCriterion;
  SolverPtr conditionLearner;
  size_t minExamplesToSplit;
  size_t maxDepth;

  bool isInBatch;

  bool isConstant(const VectorPtr& data, const IndexSetPtr& indices) const
  {
    if (indices->size() <= 1)
      return true;
    
    if (data.isInstanceOf<DVector>())
    {
      DVectorPtr d = data.staticCast<DVector>();
      IndexSet::const_iterator it = indices->begin();
      double value = d->get(*it);
      for (++it; it != indices->end(); ++it)
        if (fabs(value - d->get(*it)) > 1e-9)
          return false;
    }
    else if (data->getElementsType()->inheritsFrom(denseDoubleVectorClass()))
    {
      OVectorPtr o = data.staticCast<OVector>();
      IndexSet::const_iterator it = indices->begin();
      DenseDoubleVectorPtr value = o->getAndCast<DenseDoubleVector>(*it);
      for (++it; it != indices->end(); ++it)
      {
        double distance = value->l2norm(o->getAndCast<DenseDoubleVector>(*it));
        if (distance > 1e-9)
          return false;
      }
    }
    else
    {
      IndexSet::const_iterator it = indices->begin();
      ObjectPtr value = data->getElement(*it);
      for (++it; it != indices->end(); ++it)
        if (!Object::equals(value, data->getElement(*it)))
          return false;
    }
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
    // configure splitting criterion
    splittingCriterion->configure(objective->getData(), objective->getSupervision(), DenseDoubleVectorPtr(), indices);

    // min examples and max depth conditions to make a leaf
    if ((indices->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth) || isConstant(objective->getSupervisions(), indices))
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    // launch condition learner
    ProblemPtr conditionProblem = new Problem(problem->getDomain(), splittingCriterion);
    ExpressionPtr conditionNode;
    FitnessPtr conditionFitness;
    conditionLearner->solve(context, conditionProblem, storeBestSolverCallback(*(ObjectPtr* )&conditionNode, conditionFitness));
    double worstFitness, bestFitness;
    splittingCriterion->getObjectiveRange(worstFitness, bestFitness);

    // check condition and update importance values
    if (!conditionNode || conditionNode.isInstanceOf<ConstantExpression>() || fabs(conditionFitness->getValue(0) - worstFitness) < 1e-9)
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    conditionNode->addImportance(conditionFitness->getValue(0) * indices->size() / objective->getData()->getNumRows());
    if (verbosity >= verbosityDetailed)
      context.informationCallback(conditionNode->toShortString() + T(" [") + string(conditionNode->getSubNode(0)->getImportance()) + T("]"));

    // otherwise split examples...
    DataVectorPtr conditionValues = conditionNode->compute(context, objective->getData(), indices);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    TestExpression::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    if (failureExamples->size() == indices->size() || successExamples->size() == indices->size() || missingExamples->size() == indices->size())
    {
      jassertfalse; // all the examples go into one of the child branches, this should never happen
      return new ConstantExpression(splittingCriterion->computeVote(indices));
    }

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

#endif // ML_LEARNER_TREE_H_
