/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeIncrementalLearner.h  | ExtraTree Incremental Learner   |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXTRA_TREE_INCREMENTAL_LEARNER_H_
# define ML_EXTRA_TREE_INCREMENTAL_LEARNER_H_

# include <ml/Expression.h>
# include <ml/SplittingCriterion.h>
# include <ml/IncrementalLearner.h>

namespace lbcpp
{
 
class PureRandomScalarVectorTreeIncrementalLearner : public IncrementalLearner
{
public:
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return scalarVectorTreeNode();}
  
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr root, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    TreeNodePtr leaf = root.staticCast<TreeNode>()->findLeaf(input);
    leaf->addSample(input, output);
      
    size_t numAttributes = input->getNumValues(); 
    size_t testVariable = context.getRandomGenerator()->sampleSize(numAttributes);

    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;
    for (size_t i = 0; i < leaf->getNumSamples(); ++i)
    {
      double value = leaf->getSampleInput(i).staticCast<DenseDoubleVector>()->getValue(testVariable);
      if (value < minValue)
        minValue = value;
      if (value > maxValue)
        maxValue = value;
    }
    if (maxValue > minValue)
    {
      double testThreshold = context.getRandomGenerator()->sampleDouble(minValue, maxValue);
      leaf->split(context, testVariable, testThreshold);
    }
  }
};

class EnsembleIncrementalLearner : public IncrementalLearner
{
public:
  EnsembleIncrementalLearner(IncrementalLearnerPtr baseLearner, size_t ensembleSize)
    : baseLearner(baseLearner), ensembleSize(ensembleSize) {}
  EnsembleIncrementalLearner() : ensembleSize(0) {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
  {
    std::pair<AggregatorPtr, ClassPtr> aggregatorAndOutputType = Aggregator::create(supervisionType);
    AggregatorPtr aggregator = aggregatorAndOutputType.first;
    ClassPtr outputType = aggregatorAndOutputType.second;
    AggregatorExpressionPtr res = new AggregatorExpression(aggregator, outputType);
    res->reserveNodes(ensembleSize);
    for (size_t i = 0; i < ensembleSize; ++i)
      res->pushNode(baseLearner->createExpression(context, supervisionType));
    return res;
  }

  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expr) const
  {
    AggregatorExpressionPtr expression = expr.staticCast<AggregatorExpression>();
    for (size_t i = 0; i < expression->getNumSubNodes(); ++i)
      baseLearner->addTrainingSample(context, sample, expression->getSubNode(i));
  }
  
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expr, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    AggregatorExpressionPtr expression = expr.staticCast<AggregatorExpression>();
    for (size_t i = 0; i < expression->getNumSubNodes(); ++i)
      baseLearner->addTrainingSample(context, expression->getSubNode(i), input, output);
  }

protected:
  friend class EnsembleIncrementalLearnerClass;

  IncrementalLearnerPtr baseLearner;
  size_t ensembleSize;
};

}; /* namespace lbcpp */

#endif // !ML_EXTRA_TREE_INCREMENTAL_LEARNER_H_
