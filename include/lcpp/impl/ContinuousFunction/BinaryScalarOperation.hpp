/*-----------------------------------------.---------------------------------.
| Filename: BinaryScalarOperation.hpp      | R^2 -> R functions              |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_
# define LCPP_CORE_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_

namespace lcpp {
namespace impl {

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
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_FUNCTION_BINARY_SCALAR_OPERATION_H_
