/*-----------------------------------------.---------------------------------.
| Filename: SumOfSquaresScalarObjectFunction.h| f(x) = sum_i x_i^2           |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_SUM_OF_SQUARES_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_SUM_OF_SQUARES_H_

# include <lbcpp/Function/ScalarObjectFunction.h>
# include <lbcpp/Perception/PerceptionMaths.h>

namespace lbcpp
{

class SumOfSquaresScalarObjectFunction : public ScalarObjectFunction
{
public:
  virtual String toString() const
    {return "(sum_i x_i^2)";}

  virtual bool isDerivable() const
    {return true;}
  
  virtual void compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
  {
    if (output)
      *output = lbcpp::sumOfSquares(input);
    if (gradientTarget)
      lbcpp::addWeighted(*gradientTarget, input, 2.0 * gradientWeight);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_SUM_OF_SQUARES_H_
