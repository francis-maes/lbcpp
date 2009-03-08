/*-----------------------------------------.---------------------------------.
| Filename: ScalarContinuousFunction.hpp   | Continuous functions f: R -> R  |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_CONTINUOUS_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_CONTINUOUS_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = abs(x)
struct AbsoluteScalarFunction : public ScalarFunction<AbsoluteScalarFunction>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = fabs(input);
    if (derivative)
    {
      if (input > 0)
        *derivative = 1;
      else if (input < 0)
        *derivative = -1;
      else
      {
        assert(derivativeDirection);
        if (*derivativeDirection > 0)
          *derivative = 1;
        else if (*derivativeDirection < 0)
          *derivative = -1;
        else
          *derivative = 0;
      }
    }
  }
};
inline AbsoluteScalarFunction absFunction()
  {return AbsoluteScalarFunction();}

// f(x) = max(0, -x)
struct PerceptronLossFunction : public ScalarFunction<PerceptronLossFunction>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (input >= 0)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == 0)
    {
      if (output) *output = 0.0;
      if (derivative) {assert(derivativeDirection); *derivative = derivativeDirection <= 0 ? -1 : 0;}
    }
    else
    {
      if (output) *output = -input;
      if (derivative) *derivative = -1;
    }
  }
};
inline PerceptronLossFunction perceptronLossFunction()
  {return PerceptronLossFunction();}

// f(x) = max(0, 1 - input)
struct HingeLossFunction : public ScalarFunction<HingeLossFunction>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (input >= 1)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == 1)
    {
      if (output) *output = 0.0;
      if (derivative) {assert(derivativeDirection); *derivative = derivativeDirection <= 0 ? -1 : 0;}
    }
    else
    {
      if (output) *output = 1 - input;
      if (derivative) *derivative = -1;
    }
  }
};
inline HingeLossFunction hingeLossFunction()
  {return HingeLossFunction();}


}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_CONTINUOUS_H_
