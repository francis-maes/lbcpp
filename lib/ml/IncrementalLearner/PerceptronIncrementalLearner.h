/*-----------------------------------------.---------------------------------.
| Filename: PerceptronIncrementalLearner.h | Perceptron                      |
| Author  : Denny Verbeeck                 | Incremental Linear Perceptron   |
| Started : 05/12/2013 11:17               | Learner                         |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
# define ML_PERCEPTRON_INCREMENTAL_LEARNER_H_

# include <ml/IncrementalLearner.h>
# include <ml/Expression.h>
# include <ml/RandomVariable.h>

namespace lbcpp
{

class PerceptronIncrementalLearner : public IncrementalLearner
{
public:
  PerceptronIncrementalLearner(size_t numInitialTrainingSamples, double learningRate, double learningRateDecay)
    : numInitialTrainingSamples(numInitialTrainingSamples), learningRate(learningRate), learningRateDecay(learningRateDecay) {}
  PerceptronIncrementalLearner() {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
    {return new PerceptronExpression();}

  virtual void addTrainingSample(ExecutionContext &context, const std::vector<ObjectPtr>& sample, ExpressionPtr expression) const
  {
    PerceptronExpressionPtr perceptron = expression.staticCast<PerceptronExpression>();
    perceptron->updateStatisticsFromTrainingSample(context, sample);

    size_t numTrainingSamples = (size_t) perceptron->getStatistics(0)->getCount();
    if (numTrainingSamples < numInitialTrainingSamples)
      return;
    
    double curLearningRate = learningRate / (1 + numTrainingSamples * learningRateDecay);
    double prediction = Double::get(perceptron->compute(context, sample));
    double realVal = Double::get(sample.back());
    double dy = realVal - prediction;
    DenseDoubleVectorPtr& weights = perceptron->getWeights();
    DenseDoubleVectorPtr normalized = perceptron->normalizedInputVectorFromTrainingSample(sample);
    context.enterScope((string) numTrainingSamples);
    context.resultCallback("numTrainingSamples", numTrainingSamples);
    context.resultCallback("error", dy);
    context.resultCallback("mean0", perceptron->getStatistics(0)->getMean());
    context.resultCallback("stddev0", perceptron->getStatistics(0)->getStandardDeviation());
    DenseDoubleVectorPtr s = new DenseDoubleVector(sample.size() - 1, 0.0);
    for (size_t i = 0; i < sample.size() - 1; ++i)
      s->setValue(i, Double::get(sample[i+1]));
    context.resultCallback("sample", s);
    context.resultCallback("normalized sample", normalized);
    context.resultCallback("prediction", prediction);
    context.resultCallback("real value", realVal);
    context.resultCallback("current learning rate", curLearningRate);
    double update = 0.0;
    for (size_t i = 0; i < weights->getNumValues(); ++i)
    {
      weights->getValueReference(i) += curLearningRate * dy * normalized->getValue(i);
      context.resultCallback("weight" + ((string) i), weights->getValue(i));
    }
    context.leaveScope();
  }

protected:
  friend class PerceptronIncrementalLearnerClass;

  size_t numInitialTrainingSamples; /**< Learning will occur only after this amount of training samples, used to ensure proper initialization of mean and standard deviation of input variables */
  double learningRate;              /**< Learning rate for this perceptron */
  double learningRateDecay;         /**< Learning rate decay for this perceptron. */
};

} /* namespace lbcpp */

#endif //!ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
