/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeIncrementalLearner.h  | ExtraTree Incremental Learner   |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_EXTRA_TREE_INCREMENTAL_LEARNER_H_
# define MOO_EXTRA_TREE_INCREMENTAL_LEARNER_H_

# include <ml/SplittingCriterion.h>
# include <ml/IncrementalLearner.h>
# include "ScalarVectorTreeExpression.h"

namespace lbcpp
{
 
class ScalarVectorTreeIncrementalLearner : public IncrementalLearner
{
public:
  typedef std::vector<double> InputVector;
  typedef std::vector<double> Prediction;

  virtual void addSampleToTree(ExecutionContext& context, ScalarVectorTreeNode* root, const InputVector& input, const Prediction& output) const = 0;

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return new ScalarVectorTreeExpression(supervisionType);}

  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expr) const
  {
    ScalarVectorTreeExpressionPtr expression = expr.staticCast<ScalarVectorTreeExpression>();

    std::vector<double> input(sample.size() - 1);
    for (size_t i = 0; i < input.size(); ++i)
      input[i] = Double::get(sample[i]);

    std::vector<double> output;
    ObjectPtr supervision = sample.back();
    if (supervision.isInstanceOf<Double>())
      output.resize(1, Double::get(supervision));
    else
      output = supervision.staticCast<DenseDoubleVector>()->getValues();

    if (expression->hasRoot())
      addSampleToTree(context, expression->getRoot(), input, output);
    else
      expression->createRoot(input, output);
  }
};

class PureRandomScalarVectorTreeIncrementalLearner : public ScalarVectorTreeIncrementalLearner
{
public:
  virtual void addSampleToTree(ExecutionContext& context, ScalarVectorTreeNode* root, const InputVector& input, const Prediction& output) const
  {
    ScalarVectorTreeNode* leaf = root->findLeaf(input);
    leaf->addSample(input, output);
      
    size_t numAttributes = input.size(); 
    size_t testVariable = context.getRandomGenerator()->sampleSize(numAttributes);

    double minValue = DBL_MAX;
    double maxValue = -DBL_MAX;
    for (size_t i = 0; i < leaf->getNumSamples(); ++i)
    {
      double value = leaf->getSampleInput(i)[testVariable];
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

protected:
  friend class EnsembleIncrementalLearnerClass;

  IncrementalLearnerPtr baseLearner;
  size_t ensembleSize;
};

}; /* namespace lbcpp */

#endif // !MOO_SANDBOX_H_
