/*-----------------------------------------------------.---------------------------------.
| Filename: SimpleLinearRegressionIncrementalLearner.h | Simple Linear Regression        |
| Author  :                                            | Incremental Learning            |
| Started :                                            |                                 |
`------------------------------------------------------/                                 |
                               |                                                         |
                               `--------------------------------------------------------*/

#ifndef ML_SIMPLE_LINEAR_REGRESSION_INCREMENTAL_LEARNER_H_
# define ML_SIMPLE_LINEAR_REGRESSION_INCREMENTAL_LEARNER_H_

# include <ml/IncrementalLearner.h>
# include <ml/Expression.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

class SimpleLinearRegressionIncrementalLearner : public IncrementalLearner
{
public:
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
  {
    LinearModelExpressionPtr model = new LinearModelExpression();
    model->setLearnerStatistics(new MultiVariateRegressionStatistics());
    return model;
  }

  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    jassert(output->getNumValues() == 1); // only 1 dim for now
    LinearModelExpressionPtr model = expression.staticCast<LinearModelExpression>();
    size_t numAttributes = input->getNumValues();
    
    // check if model is properly initialized
    if (!model->getLearnerStatistics().dynamicCast<MultiVariateRegressionStatistics>())
      model->setLearnerStatistics(new MultiVariateRegressionStatistics());

    MultiVariateRegressionStatisticsPtr stats = model->getLearnerStatistics().staticCast<MultiVariateRegressionStatistics>();

    DenseDoubleVectorPtr& weights = model->getWeights();
    if (weights->getNumValues() != numAttributes + 1)
      weights = new DenseDoubleVector(numAttributes + 1, 0.0);

    // add training sample
    stats->push(input, output->getValue(0));

    // calculate slopes
    for (size_t i = 0; i < numAttributes; ++i)
      weights->setValue(i + 1, stats->getStats(i)->getSlope());

    // calculate intercept
    weights->setValue(0, stats->getStats(0)->getYMean());
    for (size_t i = 0; i < numAttributes; ++i)
      weights->getValueReference(0) -= weights->getValue(i+1) * stats->getStats(i)->getXMean();
  }

  /* Initialise the simple linear regression, data should be an OVector with PearsonCorrelationCoefficients as the elements, one for each attribute
   *
   */
  virtual void initialiseLearnerStatistics(ExecutionContext& context, ExpressionPtr model, ObjectPtr data) const 
  {
    model->setLearnerStatistics(data);
    MultiVariateRegressionStatisticsPtr stats = model->getLearnerStatistics().staticCast<MultiVariateRegressionStatistics>();
    DenseDoubleVectorPtr& weights = model.staticCast<LinearModelExpression>()->getWeights();

    if (weights->getNumValues() != stats->getNumAttributes() + 1)
      weights = new DenseDoubleVector(stats->getNumAttributes() + 1, 0.0);
    
    // calculate slopes
    for (size_t i = 1; i < weights->getNumValues(); ++i)
      weights->setValue(i, stats->getStats(i - 1)->getSlope());

    // calculate intercept
    weights->setValue(0, stats->getStats(0)->getYMean());
    for (size_t i = 0; i < stats->getNumAttributes(); ++i)
      weights->getValueReference(0) -= weights->getValue(i+1) * stats->getStats(i)->getXMean();
  }
};

typedef ReferenceCountedObjectPtr<SimpleLinearRegressionIncrementalLearner> SimpleLinearRegressionIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_SIMPLE_LINEAR_REGRESSION_INCREMENTAL_LEARNER_H_
