/*-----------------------------------------.---------------------------------.
| Filename: PerceptionMaths.cpp            | Perception Math Functions       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 16:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/PerceptionMaths.h>
using namespace lbcpp;

/*
** DoubleConstUnaryOperation
*/
struct DoubleConstUnaryOperation
{
  void sense(double value)
    {jassert(false);}
  void sense(PerceptionPtr perception, const Variable& input)
    {jassert(false);}
  void sense(ObjectPtr object)
    {jassert(false);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, ObjectPtr object)
{
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = object->getVariable(i);
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
}

template<class OperationType>
struct DoubleConstUnaryOperationCallback : public PerceptionCallback
{
  DoubleConstUnaryOperationCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    jassert(value.isDouble() && !value.isMissingValue());
    operation.sense(value.getDouble());
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {operation.sense(subPerception, input);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, PerceptionPtr perception, const Variable& input)
{
  typedef DoubleConstUnaryOperationCallback<OperationType> Callback;
  ReferenceCountedObjectPtr<Callback> callback(new Callback(operation));
  perception->computePerception(input, callback);
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

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l0norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l0norm(object);}
};

size_t lbcpp::l0norm(ObjectPtr object)
  {ComputeL0NormOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

size_t lbcpp::l0norm(PerceptionPtr perception, const Variable& input)
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

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l1norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l1norm(object);}
};

double lbcpp::l1norm(ObjectPtr object)
  {ComputeL1NormOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

double lbcpp::l1norm(PerceptionPtr perception, const Variable& input)
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

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::sumOfSquares(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::sumOfSquares(object);}
};

double lbcpp::sumOfSquares(ObjectPtr object)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, object); return operation.res;}

double lbcpp::sumOfSquares(PerceptionPtr perception, const Variable& input)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Dot-product
*/
struct ComputeDotProductCallback : public PerceptionCallback
{
  ComputeDotProductCallback(ObjectPtr object)
    : object(object), res(0.0) {}

  ObjectPtr object;
  double res;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    jassert(value.isDouble() && !value.isMissingValue());
    res += object->getVariable(variableNumber).getDouble() * value.getDouble();
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (subObject)
      res += dotProduct(subObject, subPerception, subInput);
  }
};

double lbcpp::dotProduct(ObjectPtr object, PerceptionPtr perception, const Variable& input)
{
  ReferenceCountedObjectPtr<ComputeDotProductCallback> callback(new ComputeDotProductCallback(object));
  perception->computePerception(input, callback);
  return callback->res;
}

/*
** DoubleAssignmentOperation
*/
struct DoubleAssignmentOperation
{
  void compute(ObjectPtr value, PerceptionPtr perception, const Variable& input)
    {jassert(false);}

  double compute(double value, double otherValue)
    {jassert(false); return 0.0;}
};

template<class OperationType>
struct DoubleAssignmentCallback : public PerceptionCallback
{
  DoubleAssignmentCallback(ObjectPtr object, OperationType& operation)
    : object(object), operation(operation) {}

  ObjectPtr object;
  OperationType& operation;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    jassert(value.isDouble() && !value.isMissingValue());
    Variable targetVariable = object->getVariable(variableNumber);
    object->setVariable(variableNumber,
      Variable(operation.compute(targetVariable.getDouble(), value.getDouble()), targetVariable.getType()));
  }

  virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (!subObject)
    {
      subObject = Variable::create(subPerception->getOutputType()).getObject();
      object->setVariable(variableNumber, subObject);
    }
    operation.compute(subObject, subPerception, subInput);
  }
};

struct AddWeightedOperation : public DoubleAssignmentOperation
{
  AddWeightedOperation(double weight)
    : weight(weight) {}

  double weight;

  void compute(ObjectPtr value, PerceptionPtr perception, const Variable& input)
    {lbcpp::addWeighted(value, perception, input, weight);}

  double compute(double value, double otherValue)
    {return value + weight * otherValue;}
};

void lbcpp::addWeighted(ObjectPtr object, PerceptionPtr perception, const Variable& input, double weight)
{
  if (weight)
  {
    AddWeightedOperation operation(weight);
    typedef DoubleAssignmentCallback<AddWeightedOperation> Callback;
    ReferenceCountedObjectPtr<Callback> callback(new Callback(object, operation));
    perception->computePerception(input, callback);
  }
}
