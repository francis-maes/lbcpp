/*-----------------------------------------.---------------------------------.
| Filename: LogBinomialLossFunction.h      | LogBinomial Loss Function       |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_LOG_BINOMIAL_LOSS_H_
# define LBCPP_FUNCTION_SCALAR_LOG_BINOMIAL_LOSS_H_

# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

// f(x) = log(1 + exp(-x))
class LogBinomialLossFunction : public BinaryClassificationLossFunction
{
public:
  LogBinomialLossFunction(bool isPositive)
    : BinaryClassificationLossFunction(isPositive) {}
  LogBinomialLossFunction() {}

  virtual bool isDerivable() const
    {return true;}

  virtual void computePositive(double input, double* output, const double* , double* derivative) const
  {
    if (input < -10) // avoid approximation errors in the exp(-x) formula
    {
      if (derivative)
        *derivative = -1;
      if (output)
        *output = -input;
      return;
    }
    
    if (input == 0)
    {
      static const double log2 = log(2.0);
      if (output)
        *output = log2;
      if (derivative)
        *derivative = -1.0 / 2.0;
      return;
    }
    
    double res = log(1 + exp(-input));
    jassert(isNumberValid(res));
    if (isNumberNearlyNull(res))
    {
      if (output)
        *output = 0.0;
      if (derivative)
        *derivative = 0.0;
      return;
    }

    if (output)
      *output = res;
    if (derivative)
    {
      *derivative = -1 / (1 + exp(input));
      jassert(isNumberValid(*derivative));
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_LOG_BINOMIAL_LOSS_H_
