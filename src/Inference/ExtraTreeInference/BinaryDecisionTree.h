/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTree.h           | A class to store a binary       |
| Author  : Francis Maes                   |  decision tree                  |
| Started : 25/06/2010 18:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
# define LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_

# include <lbcpp/Object/Object.h>

namespace lbcpp 
{

struct VariantUnion
{
  VariantUnion(bool boolValue)
    {u.boolValue = boolValue;}
  VariantUnion(int intValue)
    {u.intValue = intValue;} 
  VariantUnion(double doubleValue)
    {u.doubleValue = doubleValue;}
  VariantUnion(const String& stringValue)
    {u.stringValue = new String(stringValue);}
  VariantUnion(ObjectPtr objectValue)
  {
    u.objectValue = objectValue.get();
    if (objectValue)
      objectValue->incrementReferenceCounter();
  }
  VariantUnion(const VariantUnion& other)
    {memcpy(this, &other, sizeof (VariantUnion));}
  VariantUnion()
    {memset(this, 0, sizeof (VariantUnion));}

  void clear(ClassPtr type)
  {
    if (type)
    {
      if (type.isInstanceOf<StringClass>())
        clearString();
      else if (!type.isInstanceOf<BuiltinTypeClass>())
        clearObject();
    }
  }

  void clearObject()
  {
    if (u.objectValue)
    {
      u.objectValue->decrementReferenceCounter();
      u.objectValue = NULL;
    }
  }

  void clearString()
  {
    delete u.stringValue;
    u.stringValue = NULL;
  }

  bool getBoolean() const
    {return u.boolValue;}

  int getInteger() const
    {return u.intValue;}

  double getDouble() const
    {return u.doubleValue;}

  const String& getString() const
    {return *u.stringValue;}

  ObjectPtr getObject() const
    {return u.objectValue ? ObjectPtr(u.objectValue) : ObjectPtr();}

  ObjectPtr toObject(ClassPtr type) const
  {
    if (type.isInstanceOf<BuiltinTypeClass>())
    {
      jassert(false);
      return ObjectPtr();
    }
    else
      return getObject();
  }

private:
  union
  {
    bool boolValue;
    int intValue;
    double doubleValue;
    String* stringValue;
    Object* objectValue;
  } u;
};

class Variant
{
public:
  Variant(ClassPtr type, bool boolValue)
    : type(type), value(boolValue) {jassert(isBoolean());}
  Variant(ClassPtr type, int intValue)
    : type(type), value(intValue) {jassert(isInteger());} 
  Variant(ClassPtr type, double doubleValue)
    : type(type), value(doubleValue) {jassert(isDouble());}
  Variant(ClassPtr type, const String& stringValue)
    : type(type), value(stringValue) {jassert(isString());}
  Variant(ClassPtr type, ObjectPtr objectValue)
    : type(type), value(objectValue) {jassert(isObject());}

  Variant(const Variant& otherVariant)
    {*this = otherVariant;}
  
  ~Variant()
    {clear();}

  void clear()
    {value.clear(type); type = ClassPtr();}

  Variant& operator =(const Variant& otherVariant)
  {
    clear();
    type = otherVariant.getType();
    if (isUndefined())
      return *this;
    else if (isBoolean())
      value = VariantUnion(otherVariant.getBoolean());
    else if (isInteger())
      value = VariantUnion(otherVariant.getInteger());
    else if (isDouble())
      value = VariantUnion(otherVariant.getDouble());
    else if (isString())
      value = VariantUnion(otherVariant.getString());
    else if (isObject())
      value = VariantUnion(otherVariant.getObject());
    else
    {
      Object::error(T("Variant::operator ="), T("Unrecognized type of variant"));
      jassert(false);
    }
    return *this;
  }

  ClassPtr getType() const
    {return type;}

  bool isUndefined() const
    {return !type;}

  bool isBoolean() const
    {return type && type.isInstanceOf<BooleanClass>();}

  bool getBoolean() const
    {jassert(isBoolean()); return value.getBoolean();}

  bool isInteger() const
    {return type && type.isInstanceOf<IntegerClass>();}

  int getInteger() const
    {jassert(isInteger()); return value.getInteger();}

  bool isDouble() const
    {return type && type.isInstanceOf<DoubleClass>();}

  double getDouble() const
    {jassert(isDouble()); return value.getDouble();}

  bool isString() const
    {return type && type.isInstanceOf<StringClass>();}

  String getString() const
    {jassert(isString()); return value.getString();}

  bool isObject() const
    {return type && !type.isInstanceOf<BuiltinTypeClass>();}

  ObjectPtr getObject() const
    {jassert(isObject()); return value.getObject();}

private:
  ClassPtr type;
  VariantUnion value;
};

class BinaryDecisionTree : public Object
{
public:
  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  size_t addNode()
  {
    size_t res = nodes.size();
    nodes.push_back(Node()); // FIXME
    return res;
  }

protected:
  struct Node
  {
    size_t indexOfLeftChild;
    size_t splitVariable;
    VariantUnion splitArgument;
  };
  std::vector<Node> nodes;
};

typedef ReferenceCountedObjectPtr<BinaryDecisionTree> BinaryDecisionTreePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_BINARY_DECISION_TREE_H_
