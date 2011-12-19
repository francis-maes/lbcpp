/*-----------------------------------------.---------------------------------.
| Filename: ObjectLuapeFunctions.h         | Object Luape Functions          |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 18:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FUNCTION_OBJECT_H_
# define LBCPP_LUAPE_FUNCTION_OBJECT_H_

# include <lbcpp/Luape/LuapeFunction.h>
# include <lbcpp/Luape/LuapeNode.h>
# include <lbcpp/Luape/LuapeCache.h> // for LuapeSampleVector

namespace lbcpp
{

template<class ExactType>
class UnaryObjectLuapeFuntion : public LuapeFunction
{
public:
  UnaryObjectLuapeFuntion(ClassPtr inputClass = objectClass)
    : inputClass(inputClass) {}

  Variable computeObject(const ObjectPtr& object) const
    {jassert(false); return Variable();} // must be implemented

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(inputClass);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? pthis().computeObject(object) : Variable();
  }

  virtual LuapeSampleVectorPtr compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
  {
    LuapeSampleVectorPtr objects = inputs[0];
    size_t n = objects->size();
    if (outputType->inheritsFrom(objectClass))
    {
      ObjectVectorPtr res = new ObjectVector(outputType, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = objects->begin(); it != objects->end(); ++it, ++i)
      {
        const ObjectPtr& object = it.getRawObject();
        if (object)
          res->set(i, pthis().computeObject(object).getObject());
      }
      return new LuapeSampleVector(objects->getIndices(), res);
    }
    else if (outputType->inheritsFrom(doubleType))
    {
      DenseDoubleVectorPtr res = new DenseDoubleVector(positiveIntegerEnumerationEnumeration, outputType, n, doubleMissingValue);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = objects->begin(); it != objects->end(); ++it, ++i)
      {
        const ObjectPtr& object = it.getRawObject();
        if (object)
          res->setValue(i, pthis().computeObject(object).getDouble());
      }
      return new LuapeSampleVector(objects->getIndices(), res);
    }
    else if (outputType->inheritsFrom(booleanType))
    {
      BooleanVectorPtr res = new BooleanVector(n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = objects->begin(); it != objects->end(); ++it, ++i)
      {
        const ObjectPtr& object = it.getRawObject();
        if (object)
        {
          Variable v = pthis().computeObject(object);
          res->getData()[i] = v.isMissingValue() ? 2 : (v.getBoolean() ? 1 : 0);
        }
      }
      return new LuapeSampleVector(objects->getIndices(), res);
    }
    else
    {
      VectorPtr res = vector(outputType, n);
      size_t i = 0;
      for (LuapeSampleVector::const_iterator it = objects->begin(); it != objects->end(); ++it, ++i)
      {
        const ObjectPtr& object = it.getRawObject();
        if (object)
          res->setElement(i, pthis().computeObject(object));
      }
      return new LuapeSampleVector(objects->getIndices(), res);
    }
  }

protected:
  ClassPtr inputClass;

  const ExactType& pthis() const
    {return *(const ExactType* )this;}
};

class GetVariableLuapeFunction : public UnaryObjectLuapeFuntion<GetVariableLuapeFunction>
{
public:
  GetVariableLuapeFunction(size_t variableIndex = 0)
    : UnaryObjectLuapeFuntion<GetVariableLuapeFunction>(objectClass), variableIndex(variableIndex) {}

  virtual String toShortString() const
    {return ".var[" + String((int)variableIndex) + "]";}

  virtual TypePtr initialize(const std::vector<TypePtr>& inputTypes)
  {
    jassert(inputTypes.size() == 1);
    inputClass = inputTypes[0].staticCast<Class>();
    outputType = inputTypes[0]->getMemberVariableType(variableIndex);
    return outputType;
  }

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    VariableSignaturePtr member = inputs[0]->getType()->getMemberVariable(variableIndex);
    return inputs[0]->toShortString() + "." + (member->getShortName().isNotEmpty() ? member->getShortName() : member->getName());
  }

  Variable computeObject(const ObjectPtr& object) const
    {return inputClass->getMemberVariableValue(object.get(), variableIndex);}

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    TypePtr objectClass = inputTypes[0];
    size_t n = objectClass->getNumMemberVariables();
    VectorPtr res = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, i);
    return res;
  }

protected:
  friend class GetVariableLuapeFunctionClass;
  size_t variableIndex;

  ClassPtr inputClass;
  TypePtr outputType;
};

class GetContainerLengthLuapeFunction : public UnaryObjectLuapeFuntion<GetContainerLengthLuapeFunction>
{
public:
  GetContainerLengthLuapeFunction() : UnaryObjectLuapeFuntion<GetContainerLengthLuapeFunction>(containerClass()) {}

  virtual String toShortString() const
    {return "length(.)";}

  virtual TypePtr initialize(const std::vector<TypePtr>& inputTypes)
    {return positiveIntegerType;}

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
    {jassert(inputs.size() == 1); return "length(" + inputs[0]->toShortString() + ")";}

  Variable computeObject(const ObjectPtr& object) const
    {return Variable(object.staticCast<Container>()->getNumElements(), positiveIntegerType);} 

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(positiveIntegerType);
  }
};

class GetDoubleVectorElementLuapeFunction : public UnaryObjectLuapeFuntion<GetDoubleVectorElementLuapeFunction>
{
public:
  GetDoubleVectorElementLuapeFunction(size_t index = 0)
    : UnaryObjectLuapeFuntion<GetDoubleVectorElementLuapeFunction>(doubleVectorClass()), index(index) {}

  virtual String toShortString() const
    {return "[" + String((int)index) + "]";}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(type);
    return features && features->getNumElements() > 0;
  }

  virtual TypePtr initialize(const std::vector<TypePtr>& inputTypes)
  {
    EnumerationPtr features;
    DoubleVector::getTemplateParameters(defaultExecutionContext(), inputTypes[0], features, outputType);
    return outputType;
  }

  virtual String toShortString(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputs[0]->getType());
    jassert(features);
    return inputs[0]->toShortString() + "." + features->getElementName(index);
  }

  Variable computeObject(const ObjectPtr& input) const
  {
    DenseDoubleVectorPtr denseInput = input.dynamicCast<DenseDoubleVector>();
    if (denseInput)
      return denseInput->getValue(index);
    else
      return input.staticCast<DoubleVector>()->getElement(index);
  }

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(outputType);
  }

  virtual ContainerPtr getVariableCandidateValues(size_t index, const std::vector<TypePtr>& inputTypes) const
  {
    EnumerationPtr features = DoubleVector::getElementsEnumeration(inputTypes[0]);
    if (!features || features->getNumElements() == 0)
      return ContainerPtr();

    size_t n = features->getNumElements();
    VectorPtr res = vector(positiveIntegerType, n);
    for (size_t i = 0; i < n; ++i)
      res->setElement(i, i);
    return res;
  }

protected:
  friend class GetDoubleVectorElementLuapeFunctionClass;
  size_t index;

  TypePtr outputType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_OBJECT_H_
