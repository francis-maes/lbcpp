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
    {return new NormalizedLinearModelExpression();}

  virtual void addTrainingSample(ExecutionContext &context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const
  {
    // only single-objective for now
    jassert(output->getNumValues() == 1);
    NormalizedLinearModelExpressionPtr perceptron = expression.staticCast<NormalizedLinearModelExpression>();
    perceptron->updateStatistics(context, input);

    size_t numTrainingSamples = perceptron->getExamplesSeen();
    if ((size_t)perceptron->getStatistics(0)->getCount() < numInitialTrainingSamples) // if the statistics have not seen enough examples, don't learn yet
      return;
    
    double curLearningRate = learningRate / (1 + numTrainingSamples * learningRateDecay);
    double prediction = perceptron->compute(input);
    double realVal = output->getValue(0);
    double dy = realVal - prediction;
    DenseDoubleVectorPtr& weights = perceptron->getWeights();
    DenseDoubleVectorPtr normalized = perceptron->normalizeInput(input);
    weights->getValueReference(0) += curLearningRate * dy; // * 1.0
    for (size_t i = 0; i < normalized->getNumValues(); ++i)
      weights->getValueReference(i+1) += curLearningRate * dy * normalized->getValue(i);
  }

protected:
  friend class PerceptronIncrementalLearnerClass;

  size_t numInitialTrainingSamples; /**< Learning will occur only after this amount of training samples, used to ensure proper initialization of mean and standard deviation of input variables */
  double learningRate;              /**< Learning rate for this perceptron */
  double learningRateDecay;         /**< Learning rate decay for this perceptron. */
};

} /* namespace lbcpp */

#endif //!ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
