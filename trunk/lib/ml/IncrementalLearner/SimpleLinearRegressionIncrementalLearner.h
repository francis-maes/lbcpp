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
    model->setLearnerStatistics(new OVector(0));
    return model;
  }

  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    LinearModelExpressionPtr model = expression.staticCast<LinearModelExpression>();
    size_t numAttributes = input->getNumValues();
    
    // check if model is properly initialized
    if (!model->getLearnerStatistics().dynamicCast<OVector>())
      model->setLearnerStatistics(new OVector());

    OVectorPtr stats = model->getLearnerStatistics().staticCast<OVector>();
    if (stats->getNumElements() != numAttributes)
    {
      stats->resize(numAttributes);
      for (size_t i = 0; i < numAttributes; ++i)
        stats->setElement(i, new PearsonCorrelationCoefficient());
    }
    DenseDoubleVectorPtr& weights = model->getWeights();
    if (weights->getNumValues() != numAttributes + 1)
      weights = new DenseDoubleVector(numAttributes + 1, 0.0);

    // add training sample
    for (size_t i = 0; i < numAttributes; ++i)
      stats->getAndCast<PearsonCorrelationCoefficient>(i)->push(input->getValue(i), output->getValue(0));

    // calculate slopes
    for (size_t i = 0; i < numAttributes; ++i)
      weights->setValue(i + 1, stats->getAndCast<PearsonCorrelationCoefficient>(i)->getSlope());

    // calculate intercept
    weights->setValue(0, stats->getAndCast<PearsonCorrelationCoefficient>(0)->getYMean());
    for (size_t i = 0; i < numAttributes; ++i)
      weights->getValueReference(0) -= weights->getValue(i+1) * stats->getAndCast<PearsonCorrelationCoefficient>(i)->getXMean();
  }

  /* Initialise the simple linear regression, data should be an OVector with PearsonCorrelationCoefficients as the elements, one for each attribute
   *
   */
  virtual void initialiseLearnerStatistics(ExecutionContext& context, ExpressionPtr model, ObjectPtr data) const 
  {
    model->setLearnerStatistics(data);
    OVectorPtr stats = model->getLearnerStatistics().staticCast<OVector>();
    DenseDoubleVectorPtr& weights = model.staticCast<LinearModelExpression>()->getWeights();

    if (weights->getNumValues() != stats->getNumElements() + 1)
      weights = new DenseDoubleVector(stats->getNumElements() + 1, 0.0);
    
    // calculate slopes
    for (size_t i = 1; i < weights->getNumValues(); ++i)
      weights->setValue(i, stats->getAndCast<PearsonCorrelationCoefficient>(i - 1)->getSlope());

    // calculate intercept
    weights->setValue(0, stats->getAndCast<PearsonCorrelationCoefficient>(0)->getYMean());
    for (size_t i = 0; i < stats->getNumElements(); ++i)
      weights->getValueReference(0) -= weights->getValue(i+1) * stats->getAndCast<PearsonCorrelationCoefficient>(i)->getXMean();
  }
};

typedef ReferenceCountedObjectPtr<SimpleLinearRegressionIncrementalLearner> SimpleLinearRegressionIncrementalLearnerPtr;

} /* namespace lbcpp */

#endif //!ML_SIMPLE_LINEAR_REGRESSION_INCREMENTAL_LEARNER_H_
