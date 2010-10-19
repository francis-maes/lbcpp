/*-----------------------------------------.---------------------------------.
| Filename: LogBinomialMultiClassLossFu...h| Multi-class Log Binomial Loss   |
| Author  : Francis Maes                   |                                 |
| Started : 16/10/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_LOG_BINOMIAL_MULTI_CLASS_LOSS_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_LOG_BINOMIAL_MULTI_CLASS_LOSS_H_

# include <lbcpp/Function/ScalarObjectFunction.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class LogBinomialMultiClassLossFunction : public MultiClassLossFunction
{
public:
  LogBinomialMultiClassLossFunction(EnumerationPtr classes, size_t correctClass)
    : MultiClassLossFunction(classes, correctClass) {}
  LogBinomialMultiClassLossFunction() {}

  virtual bool isDerivable() const
    {return true;}

  virtual void compute(const std::vector<double>* input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const
  {
    size_t numClasses = classes->getNumElements();
    double logZ = input ? computeLogSumOfExponentials(*input) : log((double)numClasses);
    if (!isNumberValid(logZ))
      MessageCallback::error(T("LogBinomialMultiClassLossFunction::compute"), T("LogZ is not a valid number."));
    if (output)
      *output = logZ - (input ? (*input)[correctClass] : 0.0);
    if (gradientTarget)
    {
      for (size_t i = 0; i < numClasses; ++i)
      {
        double score = input ? (*input)[i] : 0.0;
        double derivative = exp(score - logZ);
        jassert(isNumberValid(derivative));
        (*gradientTarget)[i] += derivative * gradientWeight;
      }
      (*gradientTarget)[correctClass] -= gradientWeight;
    }
  }

private:
  // compute log(sum_i(exp(value[i]))) by avoiding numerical errors
  double computeLogSumOfExponentials(const std::vector<double>& values) const
  {
    double highestValue = -DBL_MAX;
    for (size_t i = 0; i < values.size(); ++i)
      if (values[i] > highestValue)
        highestValue = values[i];
    double res = 0.0;
    for (size_t i = 0; i < values.size(); ++i)
      res += exp(values[i] - highestValue);
    return log(res) + highestValue;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_LOG_BINOMIAL_MULTI_CLASS_LOSS_H_
