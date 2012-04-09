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
class UnaryObjectLuapeFunction : public LuapeFunction
{
public:
  UnaryObjectLuapeFunction(ClassPtr inputClass = objectClass)
    : inputClass(inputClass) {}

  Variable computeObject(const ObjectPtr& object) const
    {jassert(false); return Variable();} // must be implemented

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type->inheritsFrom(inputClass ? inputClass : objectClass);}

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

class GetVariableLuapeFunction : public UnaryObjectLuapeFunction<GetVariableLuapeFunction>
{
public:
  GetVariableLuapeFunction(ClassPtr inputClass = ClassPtr(), size_t variableIndex = 0)
    : UnaryObjectLuapeFunction<GetVariableLuapeFunction>(inputClass), inputClass(inputClass), variableIndex(variableIndex) {}
  GetVariableLuapeFunction(ClassPtr inputClass, const String& variableName)
    : UnaryObjectLuapeFunction<GetVariableLuapeFunction>(inputClass), inputClass(inputClass), variableIndex((size_t)inputClass->findMemberVariable(variableName))
  {
    jassert(variableIndex != (size_t)-1);
  }

  virtual String toShortString() const
    {return inputClass ? "." + inputClass->getMemberVariableName(variableIndex) : ".[" + String((int)variableIndex) + T("]");}

  virtual TypePtr initialize(const TypePtr* inputTypes)
  {
    jassert(inputTypes[0] == inputClass);
    outputType = inputClass->getMemberVariableType(variableIndex);
    return outputType;
  }

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
  {
    jassert(inputs.size() == 1);
    VariableSignaturePtr member = inputClass->getMemberVariable(variableIndex);
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
    if (index == 0)
    {
      VectorPtr res = vector(typeClass, 1);
      res->setElement(0, inputTypes[0]);
      return res;
    }
    else
    {
      TypePtr objectClass = inputTypes[0];
      size_t n = objectClass->getNumMemberVariables();
      VectorPtr res = vector(positiveIntegerType, n);
      for (size_t i = 0; i < n; ++i)
        res->setElement(i, i);
      return res;
    }
  }

  size_t getVariableIndex() const
    {return variableIndex;}

protected:
  friend class GetVariableLuapeFunctionClass;
  ClassPtr inputClass;
  size_t variableIndex;

  String variableName;
  TypePtr outputType;
};

typedef ReferenceCountedObjectPtr<GetVariableLuapeFunction> GetVariableLuapeFunctionPtr;

class GetContainerLengthLuapeFunction : public UnaryObjectLuapeFunction<GetContainerLengthLuapeFunction>
{
public:
  GetContainerLengthLuapeFunction() : UnaryObjectLuapeFunction<GetContainerLengthLuapeFunction>(containerClass()) {}

  virtual String toShortString() const
    {return "length(.)";}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return positiveIntegerType;}

  virtual String makeNodeName(const std::vector<LuapeNodePtr>& inputs) const
    {jassert(inputs.size() == 1); return "length(" + inputs[0]->toShortString() + ")";}

  Variable computeObject(const ObjectPtr& object) const
    {return Variable(object.staticCast<Container>()->getNumElements(), positiveIntegerType);} 

  virtual Variable compute(ExecutionContext& context, const Variable* inputs) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    return object ? computeObject(object) : Variable::missingValue(positiveIntegerType);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FUNCTION_OBJECT_H_
