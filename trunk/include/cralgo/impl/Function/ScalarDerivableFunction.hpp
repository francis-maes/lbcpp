/*-----------------------------------------.---------------------------------.
| Filename: ScalarDerivableFunction.hpp    | Derivable functions f: R -> R   |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_DERIVABLE_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_DERIVABLE_H_

# include "ScalarContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = x
struct IdentityScalarFunction : public ScalarFunction<IdentityScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output) *output = input;
    if (derivative) *derivative = 1.0;
  }
};
inline IdentityScalarFunction identityScalarFunction()
  {return IdentityScalarFunction();}
  
// f(x) = x * k, k in R
struct MultiplyByConstantScalarFunction : public ScalarFunction<MultiplyByConstantScalarFunction>
{
  MultiplyByConstantScalarFunction(double constant)
    : constant(constant) {}
    
  double constant;
  
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output) *output = input * constant;
    if (derivative) *derivative = constant;
  }
};
inline MultiplyByConstantScalarFunction multiplyScalarByConstant(double constant)
  {return MultiplyByConstantScalarFunction(constant);}


// f(x) = x^2
struct SquareScalarFunction : public ScalarFunction<SquareScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output)
      *output = input * input;
    if (derivative)
      *derivative = 2 * input;
  }
};
inline SquareScalarFunction squareFunction()
  {return SquareScalarFunction();}
  

// f(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
// f(x) in [-1, 1]
struct TanhScalarFunction : public ScalarFunction<TanhScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double ex = exp(input);
    double exm = exp(-input);
    double f = (ex - exm) / (ex + exm);
    if (output)
      *output = f;
    if (derivative)
      *derivative = 1 - f * f;
  }
};
inline TanhScalarFunction tanhFunction()
  {return TanhScalarFunction();}


// f(x) = 2.0 / (1.0 + exp(-x)) - 1
// f(x) in [-1, 1]
struct SigmoidScalarFunction : public ScalarFunction<SigmoidScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double s = 1.0 / (1.0 + exp(-input));
    double ss = 2 * s;
    if (output)
      *output = ss - 1;
    if (derivative)
      *derivative = ss * (1 - s);
  }
};
inline SigmoidScalarFunction sigmoidFunction()
  {return SigmoidScalarFunction();}


// f(x) = exp(-x)
// f(x) > 0
struct ExponentialLossFunction : public ScalarFunction<ExponentialLossFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double e = std::exp(-input);
    if (isLossNearlyNull(e))
    {
      if (output)
        *output = 0;
      if (derivative)
        *derivative = 0;
    }
    else
    {
      if (derivative)
        *derivative = -e; 
      if (output)
        *output = e;
    }
  }
};
inline ExponentialLossFunction exponentialLossFunction()
  {return ExponentialLossFunction();}


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
    if (isLossNearlyNull(res))
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

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_CONTINUOUS_H_
