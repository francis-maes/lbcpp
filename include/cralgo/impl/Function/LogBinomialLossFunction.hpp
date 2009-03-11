/*-----------------------------------------.---------------------------------.
| Filename: LogBinomialLossFuntion.hpp     | Log-binomial loss               |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_LOG_BINOMIAL_LOSS_H_
# define CRALGO_IMPL_FUNCTION_LOG_BINOMIAL_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = log(1 + exp(-x))
// f(x) > 0
struct LogBinomialLossFunction : public ScalarFunction<LogBinomialLossFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
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
    assert(isNumberValid(res));
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
      assert(isNumberValid(*derivative));
    }
  }
};
inline LogBinomialLossFunction logBinomialLossFunction()
  {return LogBinomialLossFunction();}

STATIC_DISCRIMINANT_LOSS_FUNCTION(logBinomialLoss, LogBinomialLoss, LogBinomialLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_LOG_BINOMIAL_LOSS_H_
