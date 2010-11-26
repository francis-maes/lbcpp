/*-----------------------------------------.---------------------------------.
| Filename: DoubleDotProductOperation.h    | Double Dot Product Operation    |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 15:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_DOT_PRODUCT_OPERATION_H_
# define LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_DOT_PRODUCT_OPERATION_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include "../../Core/Object/DenseObjectObject.h"
# include "../../Core/Object/DenseDoubleObject.h"
# include "../../Core/Object/SparseDoubleObject.h"

namespace lbcpp
{

struct DefaultComputeDotProductCallback : public PerceptionCallback
{
  DefaultComputeDotProductCallback(ExecutionContext& context, const ObjectPtr& object)
    : PerceptionCallback(context), object(object), res(0.0) {}

  ObjectPtr object;
  double res;

  virtual void sense(size_t variableNumber, double value)
  {
    if (value)
    {
      Variable objectValue = object->getVariable(variableNumber);
      if (objectValue.exists())
        res += objectValue.getDouble() * value;
    }
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    Variable objectValue = object->getVariable(variableNumber);
    if (objectValue.exists())
      res += dotProduct(context, objectValue.getObject(), value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (subObject)
      res += dotProduct(context, subObject, subPerception, subInput);
  }

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.exists())
    {
      if (value.isObject())
        sense(variableNumber, value.getObject());
      else
        sense(variableNumber, value.getDouble());
    }
  }
};

struct ComputeDotProductWithDenseObjectCallback : public PerceptionCallback
{
  ComputeDotProductWithDenseObjectCallback(ExecutionContext& context, DenseObjectObject* object)
    : PerceptionCallback(context), object(object), res(0.0) {}

  DenseObjectObject* object;
  double res;

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    const ObjectPtr& objectValue = object->getObject(variableNumber);
    if (objectValue)
      res += dotProduct(context, objectValue, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    const ObjectPtr& subObject = object->getObject(variableNumber);
    if (subObject)
      res += dotProduct(context, subObject, subPerception, subInput);
  }

  virtual void sense(size_t variableNumber, const Variable& value)
    {sense(variableNumber, value.getObject());}
};

struct ComputeDotProductWithDenseDoubleCallback : public PerceptionCallback
{
  ComputeDotProductWithDenseDoubleCallback(ExecutionContext& context, DenseDoubleObject* object)
    : PerceptionCallback(context), object(object), res(0.0) {}

  DenseDoubleObject* object;
  double res;

  virtual void sense(size_t variableNumber, double value)
  {
    if (value)
    {
      double objectValue = object->getValue(variableNumber);
      if (!object->isMissing(objectValue))
        res += objectValue * value;
    }
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const Variable& value)
    {sense(variableNumber, value.getDouble());}
};

double dotProduct(ExecutionContext& context, const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input)
{
  if (!context.checkInheritance(input.getType(), perception->getInputType()) ||
      !context.checkInheritance((TypePtr)object->getClass(), perception->getOutputType()))
    return 0.0;

  jassert(input.exists());
  if (!object)
    return 0.0;

  DenseDoubleObject* denseDoubleObject = dynamic_cast<DenseDoubleObject* >(object.get());
  if (denseDoubleObject)
  {
    ComputeDotProductWithDenseDoubleCallback callback(context, denseDoubleObject);
    perception->computePerception(context, input, &callback);
    return callback.res;
  }

  DenseObjectObject* denseObjectObject = dynamic_cast<DenseObjectObject* >(object.get());
  if (denseObjectObject)
  {
    ComputeDotProductWithDenseObjectCallback callback(context, denseObjectObject);
    perception->computePerception(context, input, &callback);
    return callback.res;
  }

  {
    DefaultComputeDotProductCallback callback(context, object);
    perception->computePerception(context, input, &callback);
    return callback.res;
  }
}

double dotProduct(const DenseDoubleObjectPtr& dense, const SparseDoubleObjectPtr& sparse)
{
  const std::vector<double>& denseValues = ((DenseDoubleObject const * )dense.get())->getValues();
  const std::vector< std::pair<size_t, double> >& sparseValues = sparse->getValues();
  double res = 0.0;
  for (size_t i = 0; i < sparseValues.size(); ++i)
  {
    const std::pair<size_t, double>& sv = sparseValues[i];
    if (sv.first >= denseValues.size())
      break;
    res += sv.second * denseValues[sv.first];
  }
  return res;
}

bool dotProductSpecialImplementation(const ObjectPtr& object1, const ObjectPtr& object2, double& res)
{
  DenseDoubleObjectPtr denseDouble1 = object1.dynamicCast<DenseDoubleObject>();
  SparseDoubleObjectPtr sparseDouble2 = object2.dynamicCast<SparseDoubleObject>();
  if (denseDouble1 && sparseDouble2)
  {
    res = dotProduct(denseDouble1, sparseDouble2);
    return true;
  }
  return false;
}

// the sparse object should be object2 for optimal performances
double dotProduct(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2)
{
  jassert(object1->getClass() == object2->getClass());
  double res = 0.0;
  if (dotProductSpecialImplementation(object1, object2, res) ||
      dotProductSpecialImplementation(object2, object1, res))
    return res;

  Object::VariableIterator* iterator = object2->createVariablesIterator();
  while (iterator->exists())
  {
    size_t index;
    Variable v2 = iterator->getCurrentVariable(index);
    if (v2.exists())
    {
      Variable v1 = object1->getVariable(index);
      if (v1.exists())
      {
        if (v1.isObject())
          res += dotProduct(v1.getObject(), v2.getObject());
        else
        {
          jassert(v1.isDouble());
          res += v1.getDouble() * v2.getDouble();
        }
      }
    }
    iterator->next();
  }
  delete iterator;
  return res;
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_DOT_PRODUCT_OPERATION_H_
