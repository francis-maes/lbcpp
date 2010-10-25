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
# include "../../Data/Object/DenseObjectObject.h"
# include "../../Data/Object/DenseDoubleObject.h"

namespace lbcpp
{

struct DefaultComputeDotProductCallback : public PerceptionCallback
{
  DefaultComputeDotProductCallback(const ObjectPtr& object)
    : object(object), res(0.0) {}

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
      res += dotProduct(objectValue.getObject(), value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (subObject)
      res += dotProduct(subObject, subPerception, subInput);
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
  ComputeDotProductWithDenseObjectCallback(DenseObjectObject* object)
    : object(object), res(0.0) {}

  DenseObjectObject* object;
  double res;

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    const ObjectPtr& objectValue = object->getObject(variableNumber);
    if (objectValue)
      res += dotProduct(objectValue, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    const ObjectPtr& subObject = object->getObject(variableNumber);
    if (subObject)
      res += dotProduct(subObject, subPerception, subInput);
  }

  virtual void sense(size_t variableNumber, const Variable& value)
    {sense(variableNumber, value.getObject());}
};

struct ComputeDotProductWithDenseDoubleCallback : public PerceptionCallback
{
  ComputeDotProductWithDenseDoubleCallback(DenseDoubleObject* object)
    : object(object), res(0.0) {}

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

double dotProduct(const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input)
{
  jassert(input.exists());
  if (!object)
    return 0.0;

  DenseDoubleObject* denseDoubleObject = dynamic_cast<DenseDoubleObject* >(object.get());
  if (denseDoubleObject)
  {
    ComputeDotProductWithDenseDoubleCallback callback(denseDoubleObject);
    perception->computePerception(input, &callback);
    return callback.res;
  }

  DenseObjectObject* denseObjectObject = dynamic_cast<DenseObjectObject* >(object.get());
  if (denseObjectObject)
  {
    ComputeDotProductWithDenseObjectCallback callback(denseObjectObject);
    perception->computePerception(input, &callback);
    return callback.res;
  }

  {
    DefaultComputeDotProductCallback callback(object);
    perception->computePerception(input, &callback);
    return callback.res;
  }
}

double dotProduct(const ObjectPtr& object1, const ObjectPtr& object2)
{
  jassert(object1->getClass() == object2->getClass());
  
  double res = 0.0;
  size_t n = object1->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v1 = object1->getVariable(i);
    Variable v2 = object2->getVariable(i);
    if (v1.exists() && v2.exists())
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
  return res;
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_DOT_PRODUCT_OPERATION_H_
