/*-----------------------------------------.---------------------------------.
| Filename: VariableReference.hpp          | Reference to a C++ native       |
| Author  : Francis Maes                   |  variable                       |
| Started : 09/07/2010 17:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VARIABLE_REFERENCE_HPP_
# define LBCPP_OBJECT_VARIABLE_REFERENCE_HPP_

namespace lbcpp
{

class VariableReference
{
public:
  VariableReference(bool& boolPointer)
    : t(boolType) {u.boolPointer = &boolPointer;}
  VariableReference(juce::int32& intPointer)
    : t(int32Type) {u.int32Pointer = &intPointer;}
  VariableReference(juce::int64& intPointer)
    : t(int64Type) {u.int64Pointer = &intPointer;}
  VariableReference(size_t& sizePointer)
    : t(sizeType)  {u.sizePointer = &sizePointer;}
  VariableReference(double& doublePointer)
    : t(doubleType) {u.doublePointer = &doublePointer;}
  VariableReference(String& stringPointer)
    : t(stringType) {u.stringPointer = &stringPointer;}
  VariableReference(ObjectPtr& objectPointer)
    : t(objectType) {u.objectPointer = &objectPointer;}

  template<class T>
  VariableReference(ReferenceCountedObjectPtr<T>& objectPointer)
    : t(objectType) {u.objectPointer = (ObjectPtr* )(&objectPointer);}

  VariableReference(Variable& variablePointer)
    : t(variableType) {u.variablePointer = &variablePointer;}
  VariableReference(const VariableReference& other)
    : t(other.t), u(other.u) {}
  VariableReference() : t(nullType) {}

  Variable get(TypePtr type, ErrorHandler& callback = ErrorHandler::getInstance()) const
  {
    switch (t)
    {
    case nullType: return Variable::missingValue(type);
    case boolType: return Variable(*u.boolPointer, type);
    case int32Type: return Variable(*u.int32Pointer, type);
    case int64Type: return Variable(*u.int64Pointer, type);
    case sizeType: return Variable(*u.sizePointer, type);
    case doubleType: return Variable(*u.doublePointer, type);
    case stringType: return Variable(*u.stringPointer, type);
    case objectType:
      {
        ObjectPtr object = *u.objectPointer;
        return object && checkInheritance((TypePtr)object->getClass(), type, callback) ? Variable(object) : Variable::missingValue(type);
      }

    case variableType:
      {
        Variable res = *u.variablePointer;
        return checkInheritance(res.getType(), type, callback) ? res : Variable::missingValue(type);
      }
    default:
      jassert(false);
      return Variable();
    };
  }

  void set(const Variable& value, ErrorHandler& callback = ErrorHandler::getInstance())
  {
    switch (t)
    {
    case nullType: jassert(false); break;
    case boolType:
      if (checkInheritance(value, lbcpp::booleanType(), callback))
        *u.boolPointer = value.getBoolean();
      break;

    case int32Type:
      if (checkInheritance(value, lbcpp::integerType(), callback))
        *u.int32Pointer = value.getInteger();
      break;

    case int64Type:
      if (checkInheritance(value, lbcpp::integerType(), callback))
        *u.int64Pointer = value.getInteger();
      break;

    case sizeType:
      if (checkInheritance(value, lbcpp::integerType(), callback))
        *u.sizePointer = (size_t)value.getInteger();
      break;

    case doubleType:
      if (checkInheritance(value, lbcpp::doubleType(), callback))
        *u.doublePointer = value.getDouble();
      break;

    case stringType:
      if (checkInheritance(value, lbcpp::stringType(), callback))
        *u.stringPointer = value.getString();
      break;

    case objectType:
      if (checkInheritance(value, lbcpp::objectClass(), callback))
        *u.objectPointer = value.getObject();
      break;

    case variableType:
      *u.variablePointer = value;
      break;

    default: jassert(false);
    };
  }

private:
  enum Type
  {
    nullType,
    boolType,
    int32Type,
    int64Type,
    sizeType,
    doubleType,
    stringType,
    objectType,
    variableType
  };

  union Union
  {
    bool* boolPointer;
    juce::int32* int32Pointer;
    juce::int64* int64Pointer;
    size_t* sizePointer;
    double* doublePointer;
    String* stringPointer;
    ObjectPtr* objectPointer;
    Variable* variablePointer;
  };

  Type t;
  Union u;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VARIABLE_REFERENCE_HPP_
