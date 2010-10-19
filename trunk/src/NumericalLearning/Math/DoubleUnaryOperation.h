/*-----------------------------------------.---------------------------------.
| Filename: DoubleUnaryOperation.h         | Double Unary Operations:        |
| Author  : Francis Maes                   |   - multiplyByScalar            |
| Started : 19/10/2010 15:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_UNARY_OPERATION_H_
# define LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_UNARY_OPERATION_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

/*
** DoubleUnaryOperation
*/
struct DoubleUnaryOperation
{
  void compute(double& value)
    {jassert(false);}
  void compute(const ObjectPtr& object)
    {jassert(false);}
};

template<class OperationType>
void doubleUnaryOperation(OperationType& operation, const ObjectPtr& object)
{
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = object->getVariable(i);
    if (v.isMissingValue())
      continue;

    if (v.isObject())
      operation.compute(v.getObject());
    else
    {
      jassert(v.isDouble());
      double value = v.getDouble();
      operation.compute(value);
      object->setVariable(i, Variable(value, v.getType()));
    }
  }
}

/*
** MultiplyByScalar
*/
struct MultiplyByScalarOperation : public DoubleUnaryOperation
{
  MultiplyByScalarOperation(double scalar)
    : scalar(scalar) {}

  double scalar;

  void compute(double& value)
    {value *= scalar;}

  void compute(const ObjectPtr& object)
    {multiplyByScalar(object, scalar);}
};

void multiplyByScalar(const ObjectPtr& object, double scalar)
{
  if (scalar != 1.0 && object)
  {
    MultiplyByScalarOperation operation(scalar);
    doubleUnaryOperation(operation, object);
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_UNARY_OPERATION_H_
