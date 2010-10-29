/*-----------------------------------------.---------------------------------.
| Filename: DoubleConstUnaryOperation.h    | Double Const Unary Operations:  |
| Author  : Francis Maes                   |   l0norm(), l1norm(), l2norm()  |
| Started : 19/10/2010 15:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_CONST_UNARY_OPERATION_H_
# define LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_CONST_UNARY_OPERATION_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include "../../Data/Object/DenseObjectObject.h"
# include "../../Data/Object/DenseDoubleObject.h"
# include "../../Data/Object/SparseDoubleObject.h"

namespace lbcpp
{

/*
** DoubleConstUnaryOperation
*/
struct DoubleConstUnaryOperation
{
  void sense(double value)
    {jassert(false);}
  void sense(const PerceptionPtr& perception, const Variable& input)
    {jassert(false);}
  void sense(const ObjectPtr& object)
    {jassert(false);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, const DenseDoubleObjectPtr& object)
{
  const std::vector<double>& values = object->getValues();
  for (size_t i = 0; i < values.size(); ++i)
    operation.sense(values[i]);
}

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, const SparseDoubleObjectPtr& object)
{
  const std::vector<std::pair<size_t, double> >& values = object->getValues();
  for (size_t i = 0; i < values.size(); ++i)
    operation.sense(values[i].second);
}

template<class OperationType>
bool doubleConstUnaryOperationSpecialImplementation(OperationType& operation, const ObjectPtr& object)
{
  SparseDoubleObjectPtr sparse = object.dynamicCast<SparseDoubleObject>();
  if (sparse)
  {
    doubleConstUnaryOperation(operation, sparse);
    return true;
  }
  DenseDoubleObjectPtr dense = object.dynamicCast<DenseDoubleObject>();
  if (dense)
  {
    doubleConstUnaryOperation(operation, dense);
    return true;
  }
  return false;
}


template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, const ObjectPtr& object)
{
  if (doubleConstUnaryOperationSpecialImplementation(operation, object))
    return;

  Object::VariableIterator* iterator = object->createVariablesIterator();
  for (; iterator->exists(); iterator->next())
  {
    size_t index;
    Variable v = iterator->getCurrentVariable(index);
    if (v.isMissingValue())
      continue;

    if (v.isObject())
      operation.sense(v.getObject());
    else
    {
      jassert(v.isDouble());
      operation.sense(v.getDouble());
    }
  }
  delete iterator;
}

template<class OperationType>
struct DoubleConstUnaryOperationCallback : public PerceptionCallback
{
  DoubleConstUnaryOperationCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, double value)
    {operation.sense(value);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {operation.sense(value);}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.isObject())
      operation.sense(value.getObject());
    else
    {
      jassert(value.isDouble() && !value.isMissingValue());
      operation.sense(value.getDouble());
    }
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
    {operation.sense(subPerception, input);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, const PerceptionPtr& perception, const Variable& input)
{
  jassert(input.exists());
  if (checkInheritance(input, perception->getInputType()))
  {
    typedef DoubleConstUnaryOperationCallback<OperationType> Callback;
    Callback callback(operation);
    perception->computePerception(input, &callback);
  }
}

/*
** L0 Norm
*/
struct ComputeL0NormOperation : public DoubleConstUnaryOperation
{
  ComputeL0NormOperation() : res(0) {}

  size_t res;

  void sense(double value)
    {if (value) ++res;}

  void sense(const PerceptionPtr& perception, const Variable& input)
    {res += lbcpp::l0norm(perception, input);}

  void sense(const ObjectPtr& object)
    {res += lbcpp::l0norm(object);}
};

size_t l0norm(const ObjectPtr& object)
{
  if (object)
  {
    ComputeL0NormOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0;
}

size_t l0norm(const PerceptionPtr& perception, const Variable& input)
  {ComputeL0NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** L1 Norm
*/
struct ComputeL1NormOperation : public DoubleConstUnaryOperation
{
  ComputeL1NormOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += fabs(value);}

  void sense(const PerceptionPtr& perception, const Variable& input)
    {res += lbcpp::l1norm(perception, input);}

  void sense(const ObjectPtr& object)
    {res += lbcpp::l1norm(object);}
};

double l1norm(const ObjectPtr& object)
{
  if (object)
  {
    ComputeL1NormOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0.0;
}

double l1norm(const PerceptionPtr& perception, const Variable& input)
  {ComputeL1NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Sum of squares
*/
struct ComputeSumOfSquaresOperation : public DoubleConstUnaryOperation
{
  ComputeSumOfSquaresOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += value * value;}

  void sense(const PerceptionPtr& perception, const Variable& input)
    {res += lbcpp::sumOfSquares(perception, input);}

  void sense(const ObjectPtr& object)
    {res += lbcpp::sumOfSquares(object);}
};

double sumOfSquares(const ObjectPtr& object)
{
  if (object)
  {
    ComputeSumOfSquaresOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0.0;
}

double sumOfSquares(const PerceptionPtr& perception, const Variable& input)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_CONST_UNARY_OPERATION_H_
