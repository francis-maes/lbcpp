/*-----------------------------------------.---------------------------------.
| Filename: LogBinomialMultiClassLossFu...h| Multi-class Log Binomial Loss   |
| Author  : Francis Maes                   |                                 |
| Started : 16/10/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_MULTI_CLASS_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_MULTI_CLASS_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class LogBinomialMultiClassLossFunction : public MultiClassLossFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    size_t numClasses = getNumClasses();
    size_t correctClass = getCorrectClass(otherInputs);
    double logZ = input ? input->computeLogSumOfExponentials() : log((double)numClasses);
    jassert(isNumberValid(logZ));
    if (output)
      *output += logZ - (input ? input->getValue(correctClass) : 0.0);
    if (gradientTarget)
    {
      for (size_t i = 0; i < numClasses; ++i)
      {
        double score = input ? input->getValue(i) : 0.0;
        double derivative = exp(score - logZ);
        jassert(isNumberValid(derivative));
        (*gradientTarget)->incrementValue(i, derivative * gradientWeight);
      }
      (*gradientTarget)->incrementValue(correctClass, -gradientWeight);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_MULTI_CLASS_H_
