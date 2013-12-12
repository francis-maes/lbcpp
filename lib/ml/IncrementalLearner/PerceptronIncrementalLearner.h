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
    double dy = Double::get(perceptron->compute(context, sample)) - Double::get(sample.back());
    std::vector<double>& weights = perceptron->getWeights();
    weights[0] += curLearningRate * dy;
    std::vector<double> normalized = perceptron->normalizedInputVectorFromTrainingSample(sample);
    for (size_t i = 1; i < weights.size(); ++i)
      weights[i] += curLearningRate * dy * normalized[i-1];
  }

protected:
  friend class PerceptronIncrementalLearnerClass;

  size_t numInitialTrainingSamples; /**< Learning will occur only after this amount of training samples, used to ensure proper initialization of mean and standard deviation of input variables */
  double learningRate;              /**< Learning rate for this perceptron */
  double learningRateDecay;         /**< Learning rate decay for this perceptron. */
};

} /* namespace lbcpp */

#endif //!ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
