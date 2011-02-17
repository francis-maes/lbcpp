/*-----------------------------------------.---------------------------------.
| Filename: LogBinomialLossFunction.h      | LogBinomial Loss Function       |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 16:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_H_

# include <lbcpp/NumericalLearning/LossFunctions.h>

namespace lbcpp
{

// f(x) = log(1 + exp(-x))
class LogBinomialDiscriminativeLossFunction : public DiscriminativeLossFunction
{
public:
  virtual bool isDerivable() const
    {return true;}

  virtual void computeDiscriminativeLoss(double score, double* output, double* derivative) const
  {
    if (score < -10) // avoid approximation errors in the exp(-x) formula
    {
      if (derivative)
        *derivative = -1;
      if (output)
        *output = -score;
      return;
    }
    
    if (score == 0)
    {
      static const double log2 = log(2.0);
      if (output)
        *output = log2;
      if (derivative)
        *derivative = -1.0 / 2.0;
      return;
    }
    
    double res = log(1 + exp(-score));
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
      *derivative = -1 / (1 + exp(score));
      jassert(isNumberValid(*derivative));
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_LOG_BINOMIAL_H_
