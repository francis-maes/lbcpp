/*-----------------------------------------.---------------------------------.
| Filename: DoubleAssignmentOperation.h    | Double Assignment Operations:   |
| Author  : Francis Maes                   |   addWeighted                   |
| Started : 19/10/2010 15:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_
# define LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

/*
** DoubleAssignmentOperation
*/
struct DoubleAssignmentOperation
{
  void compute(ObjectPtr& target, const ObjectPtr& source)
    {jassert(false);}

  void compute(ObjectPtr& value, const PerceptionPtr& perception, const Variable& input)
    {jassert(false);}

  void compute(double& value, double otherValue)
    {jassert(false);}
};

template<class OperationType>
struct DoubleAssignmentCallback : public PerceptionCallback
{
  DoubleAssignmentCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.isObject())
      sense(variableNumber, value.getObject());
    else
      sense(variableNumber, value.getDouble());
  }
}; 

template<class OperationType>
struct DefaultDoubleAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DefaultDoubleAssignmentCallback(const ObjectPtr& object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    Variable targetVariable = object->getVariable(variableNumber);
    double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
    BaseClass::operation.compute(targetValue, value);
    object->setVariable(variableNumber, Variable(targetValue, targetVariable.getType()));
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr targetObject = object->getVariable(variableNumber).getObject();
    BaseClass::operation.compute(targetObject, value);
    object->setVariable(variableNumber, targetObject);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (!subObject)
    {
      subObject = Variable::create(subPerception->getOutputType()).getObject();
      object->setVariable(variableNumber, subObject);
    }
    BaseClass::operation.compute(subObject, subPerception, subInput);
  }

  ObjectPtr object;
};

template<class OperationType>
struct DenseObjectAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DenseObjectAssignmentCallback(DenseObjectObject* object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    BaseClass::operation.compute(targetObject, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    BaseClass::operation.compute(targetObject, subPerception, subInput);
  }

  DenseObjectObject* object;
};

template<class OperationType>
struct DenseDoubleAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DenseDoubleAssignmentCallback(DenseDoubleObject* object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    double& targetValue = object->getValueReference(variableNumber);
    if (object->isMissing(targetValue))
      targetValue = 0.0;
    BaseClass::operation.compute(targetValue, value);
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
    {jassert(false);}

  DenseDoubleObject* object;
};

template<class OperationType>
void doubleAssignmentOperation(OperationType& operation, const ObjectPtr& target, const ObjectPtr& source)
{
  size_t n = source->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable sourceVariable = source->getVariable(i);
    if (sourceVariable.isMissingValue())
      continue;
    Variable targetVariable = target->getVariable(i);

    if (sourceVariable.isObject())
    {
      jassert(targetVariable.isObject());
      ObjectPtr targetObject = targetVariable.getObject();
      operation.compute(targetObject, sourceVariable.getObject());
      target->setVariable(i, targetObject);
    }
    else
    {
      jassert(sourceVariable.isDouble() && targetVariable.isDouble());
      double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
      operation.compute(targetValue, sourceVariable.getDouble());
      target->setVariable(i, Variable(targetValue, targetVariable.getType()));
    }
  }
}

/*
** AddWeighted
*/
struct AddWeightedOperation : public DoubleAssignmentOperation
{
  AddWeightedOperation(double weight)
    : weight(weight) {}

  double weight;

  void compute(ObjectPtr& target, const ObjectPtr& source)
    {lbcpp::addWeighted(target, source, weight);}

  void compute(ObjectPtr& value, const PerceptionPtr& perception, const Variable& input)
    {lbcpp::addWeighted(value, perception, input, weight);}

  void compute(double& value, double otherValue)
    {value += weight * otherValue;}
};

void lbcpp::addWeighted(ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight)
{
  if (!weight)
    return;
  if (!target)
    target = Variable::create(perception->getOutputType()).getObject();

  AddWeightedOperation operation(weight);
  DenseDoubleObject* denseDoubleTarget = dynamic_cast<DenseDoubleObject* >(target.get());
  if (denseDoubleTarget)
  {
    typedef DenseDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseDoubleTarget, operation);
    perception->computePerception(input, &callback);
    return;
  }

  DenseObjectObject* denseObjectTarget = dynamic_cast<DenseObjectObject* >(target.get());
  if (denseObjectTarget)
  {
    typedef DenseObjectAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseObjectTarget, operation);
    perception->computePerception(input, &callback);
    return;
  }

  {
    typedef DefaultDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(target, operation);
    perception->computePerception(input, &callback);
  }
}

void lbcpp::addWeighted(ObjectPtr& target, const ObjectPtr& source, double weight)
{
  if (!weight)
    return;
  if (!target)
    target = Variable::create(source->getClass()).getObject();
  if (target == source)
    lbcpp::multiplyByScalar(target, 1 + weight);
  else
  {
    AddWeightedOperation operation(weight);
    doubleAssignmentOperation(operation, target, source);
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_
