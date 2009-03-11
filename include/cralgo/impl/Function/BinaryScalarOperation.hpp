/*-----------------------------------------.---------------------------------.
| Filename: BinaryScalarOperation.hpp      | R^2 -> R functions              |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_
# define CRALGO_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_

# include <cmath>

namespace cralgo {
namespace impl {

inline bool isNumberValid(double number)
{
#ifdef WIN32
    return number == number;
#else
    return !std::isnan(number) && !std::isinf(number);
#endif
}

inline bool isNumberNearlyNull(double value)
{
  static const double epsilon = 0.00001;
  return fabs(value) < epsilon;
}

struct BinaryScalarOperation
{
  static double compute(double left, double right)
    {assert(false); return 0.0;}
  static double computeDerivativeWrtLeft(double left, double right)
    {assert(false); return 0.0;}
  static double computeDerivativeWrtRight(double left, double right)
    {assert(false); return 0.0;}
};

struct AdditionBinaryScalarOperation : public BinaryScalarOperation
{
  static double compute(double left, double right)
    {return left + right;}
  static double computeDerivativeWrtLeft(double left, double right)
    {return 1;}
  static double computeDerivativeWrtRight(double left, double right)
    {return 1;}
};

struct SubstractionBinaryScalarOperation : public BinaryScalarOperation
{
  static double compute(double left, double right)
    {return left - right;}
  static double computeDerivativeWrtLeft(double left, double right)
    {return 1;}
  static double computeDerivativeWrtRight(double left, double right)
    {return -1;}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_
