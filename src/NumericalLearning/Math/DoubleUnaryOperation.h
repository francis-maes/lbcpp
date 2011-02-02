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
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

/*
** DoubleUnaryOperation
*/
struct DoubleUnaryOperation
{
  DoubleUnaryOperation(ExecutionContext& context) : context(context) {}

  ExecutionContext& context;

  void compute(double& value)
    {jassert(false);}
  void compute(const ObjectPtr& object)
    {jassert(false);}
};

template<class OperationType>
void doubleUnaryOperation(OperationType& operation, const DenseDoubleObjectPtr& object)
{
  std::vector<double>& values = object->getValues();
  for (size_t i = 0; i < values.size(); ++i)
  {
    double& value = values[i];
    if (object->isMissing(value))
      value = 0;
    operation.compute(value);
  }
}

template<class OperationType>
void doubleUnaryOperation(OperationType& operation, const SparseDoubleObjectPtr& object)
{
  std::vector< std::pair<size_t, double> >& values = object->getValues();
  for (size_t i = 0; i < values.size(); ++i)
    operation.compute(values[i].second);
}

template<class OperationType>
bool doubleUnaryOperationSpecialImplementation(OperationType& operation, const ObjectPtr& object)
{
  DenseDoubleObjectPtr dense = object.dynamicCast<DenseDoubleObject>();
  if (dense)
  {
    doubleUnaryOperation(operation, dense);
    return true;
  }
  SparseDoubleObjectPtr sparse = object.dynamicCast<SparseDoubleObject>();
  if (sparse)
  {
    doubleUnaryOperation(operation, dense);
    return true;
  }
  return false;
}

template<class OperationType>
void doubleUnaryOperation(OperationType& operation, const ObjectPtr& object)
{
  if (doubleUnaryOperationSpecialImplementation(operation, object))
    return;

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
      double value = v.isMissingValue() ? 0.0 : v.getDouble();
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
  MultiplyByScalarOperation(ExecutionContext& context, double scalar)
    : DoubleUnaryOperation(context), scalar(scalar) {}

  double scalar;

  void compute(double& value)
    {value *= scalar;}

  void compute(const ObjectPtr& object)
    {multiplyByScalar(context, object, scalar);}
};

void multiplyByScalar(ExecutionContext& context, const ObjectPtr& object, double scalar)
{
  if (scalar != 1.0 && object)
  {
    MultiplyByScalarOperation operation(context, scalar);
    doubleUnaryOperation(operation, object);
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_UNARY_OPERATION_H_
