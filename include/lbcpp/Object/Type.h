/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: Type.h                        | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_CLASS_H_
# define LBCPP_OBJECT_CLASS_H_

# include "Object.h"
# include "impl/VariableValue.hpp"

namespace lbcpp
{

extern void initialize();

class Type : public NameableObject
{
public:
  Type(const String& className, TypePtr baseClass)
    : NameableObject(className), baseClass(baseClass) {}

  static void declare(TypePtr classInstance);
  static TypePtr get(const String& typeName);
  static TypePtr get(const String& typeName, TypePtr argument);
  static TypePtr get(const String& typeName, TypePtr argument1, TypePtr argument2);

  static bool doClassNameExists(const String& className);
  
  /** Creates dynamically an object of type @a typeName.
  **
  ** The type @a typeName must be declared with Type::declare()
  ** and must have a default constructor before being able
  ** to instantiate it dynamically.
  **
  ** @param typeName : type name.
  **
  ** @return an instance of @a typeName object.
  ** @see Type::declare
  */
  static Variable createInstance(const String& typeName);

  /*
  ** Base class
  */
  TypePtr getBaseClass() const
    {return baseClass;}

  void setBaseClass(TypePtr baseClass)
    {this->baseClass = baseClass;}

  /*
  ** Template Arguments
  */
  size_t getNumTemplateArguments() const
    {return templateArguments.size();}

  TypePtr getTemplateArgument(size_t index) const
    {jassert(index < templateArguments.size()); return templateArguments[index];}
  
  void setTemplateArgument(size_t index, TypePtr type)
    {jassert(index < templateArguments.size()); templateArguments[index] = type;}

  bool inheritsFrom(TypePtr baseType) const;

  /*
  ** Operations
  */
  virtual VariableValue create() const = 0;
  virtual void destroy(VariableValue& value) const = 0;
  virtual void copy(VariableValue& dest, const VariableValue& source) const = 0;
  virtual String toString(const VariableValue& value) const = 0;
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const = 0;

  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar)
    {jassert(false); return TypePtr(this);}

  virtual TypePtr addWeighted(VariableValue& target, const Variable& source, double weight)
    {jassert(false); return TypePtr(this);}

  /*
  ** Static Variables
  */
  virtual size_t getNumStaticVariables() const
    {return 0;}

  virtual TypePtr getStaticVariableType(size_t index) const
    {jassert(false); return TypePtr();}

  virtual String getStaticVariableName(size_t index) const
    {jassert(false); return String::empty;}

  /*
  ** Dynamic Variables
  */
  virtual size_t getNumSubVariables(const VariableValue& value) const
    {return getNumStaticVariables();}

  virtual String getSubVariableName(const VariableValue& value, size_t index) const
    {return getStaticVariableName(index);}

  virtual Variable getSubVariable(const VariableValue& value, size_t index) const;

  virtual void setSubVariable(const VariableValue& value, size_t index, const Variable& subValue) const
    {jassert(false);}

  virtual void clone(ObjectPtr target) const
  {
    TypePtr targetType = target.staticCast<Type>();
    targetType->baseClass = baseClass;
    targetType->templateArguments = templateArguments;
  }

protected:
  TypePtr baseClass;
  std::vector<TypePtr> templateArguments;
};

extern TypePtr topLevelType();
extern TypePtr nilType();

class BuiltinType : public Type
{
public:
  BuiltinType(const String& name, TypePtr baseType = topLevelType())
    : Type(name, baseType) {}
};

extern TypePtr booleanType();
extern TypePtr integerType();
extern TypePtr doubleType();
  extern TypePtr probabilityType();
  extern TypePtr angstromDistanceType(); // todo: move

extern TypePtr stringType();

extern TypePtr pairType();
extern TypePtr pairType(TypePtr firstClass, TypePtr secondClass);

/*
** Integer
*/
class IntegerType : public BuiltinType
{
public:
  IntegerType(const String& className, TypePtr baseType)
    : BuiltinType(className, baseType) {}
  IntegerType() : BuiltinType(T("Integer")) {}

  virtual void destroy(VariableValue& value) const
    {value.clearBuiltin();}

  virtual VariableValue create() const
    {return VariableValue(0);}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setInteger(source.getInteger());}

  virtual String toString(const VariableValue& value) const
    {return String(value.getInteger());}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const
    {return value1.getInteger() - value2.getInteger();}

  virtual size_t getNumSubVariables(const VariableValue& value) const
    {return 0;}
};

/*
** Enumeration
*/
class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class Enumeration : public IntegerType
{
public:
  Enumeration(const String& name, const juce::tchar** elements);
  Enumeration(const String& name, const String& elementChars);
  Enumeration(const String& name);

  static EnumerationPtr get(const String& className)
    {return checkCast<Enumeration>(T("Enumeration::get"), Type::get(className));}

  virtual String toString(const VariableValue& value) const
  {
    int val = value.getInteger();
    return val >= 0 && val < (int)getNumElements() ? getElementName(val) : T("N/A");
  }

  size_t getNumElements() const
    {return elements.size();}

  String getElementName(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar);

protected:
  void addElement(const String& elementName);

private:
  std::vector<String> elements;
};

extern TypePtr enumerationType();

/*
** Class
*/
class Class : public Type
{
public:
  Class(const String& name, TypePtr baseClass)
    : Type(name, baseClass) {}
  Class() : Type(T("Object"), topLevelType()) {}

  virtual VariableValue create() const
    {jassert(false); return VariableValue();}

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.clearObject(); dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;
  virtual TypePtr multiplyByScalar(VariableValue& value, double scalar);
  virtual TypePtr addWeighted(VariableValue& target, const Variable& source, double weight);

  virtual size_t getNumStaticVariables() const;
  virtual TypePtr getStaticVariableType(size_t index) const;
  virtual String getStaticVariableName(size_t index) const;

  int findStaticVariable(const String& name) const;

  virtual Variable getSubVariable(const VariableValue& value, size_t index) const;
  virtual void setSubVariable(const VariableValue& value, size_t index, const Variable& subValue) const;

protected:
  CriticalSection variablesLock;
  std::vector< std::pair<TypePtr, String> > variables;

  void addVariable(TypePtr type, const String& name);
  void addVariable(const String& typeName, const String& name)
    {addVariable(Type::get(typeName), name);}
};

typedef ReferenceCountedObjectPtr<Class> ClassPtr;

extern ClassPtr objectClass();

/*
** Collection
*/
class Collection;
typedef ReferenceCountedObjectPtr<Collection> CollectionPtr;

class Collection : public Class
{
public:
  Collection(const String& name)
    : Class(name, objectClass()) {}

  static CollectionPtr get(const String& className)
    {return checkCast<Collection>(T("Collection::get"), Type::get(className));}

  size_t getNumElements() const
    {return objects.size();}

  ObjectPtr getElement(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

protected:
  void addElement(ObjectPtr object)
    {objects.push_back(object);}

private:
  std::vector<ObjectPtr> objects;
};

/*
** Minimalistic C++ classes Wrapper
*/
template<class TT>
class DefaultAbstractClass_ : public Class
{
public:
  DefaultAbstractClass_(TypePtr baseClass)
    : Class(lbcpp::toString(typeid(TT)), baseClass)
    {}

  virtual VariableValue create() const
  {
    Object::error(T("AbstractClass::create"), T("Cannot instantiate abstract classes"));
    return VariableValue(0);
  }
};

template<class TT>
class DefaultClass_ : public Class
{
public:
  DefaultClass_(TypePtr baseClass, size_t numTemplateArguments = 0)
    : Class(lbcpp::toString(typeid(TT)), baseClass)
  {
    if (numTemplateArguments)
      templateArguments.resize(numTemplateArguments, topLevelType());
  }

  virtual VariableValue create() const
    {return new TT();}

  virtual ObjectPtr clone() const
  {
    ClassPtr res(new DefaultClass_<TT>(baseClass));
    Type::clone(res);
    return res;
  }
};

#define LBCPP_DECLARE_ABSTRACT_CLASS(Name, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultAbstractClass_<Name>(lbcpp::Type::get(#BaseClass))))

#define LBCPP_DECLARE_CLASS(Name, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(lbcpp::Type::get(#BaseClass))))

#define LBCPP_DECLARE_TEMPLATE_CLASS(Name, NumTemplateArguments, BaseClass) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(lbcpp::Type::get(#BaseClass), NumTemplateArguments)))

#define LBCPP_DECLARE_CLASS_LEGACY(Name) \
  lbcpp::Type::declare(lbcpp::TypePtr(new lbcpp::DefaultClass_<Name>(objectClass())))

inline bool checkInheritance(TypePtr type, TypePtr baseType)
{
  if (!type || !type->inheritsFrom(baseType))
  {
    Object::error(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
    return false;
  }
  return true;
}

/*
** TypeCache
*/
class TypeCache
{
public:
  TypeCache(const String& typeName);

  TypePtr operator ()()
    {return type;}

protected:
  TypePtr type;
};

class UnaryTemplateTypeCache
{
public:
  UnaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument);

private:
  String typeName;
  std::map<TypePtr, TypePtr> m;
};

class BinaryTemplateTypeCache
{
public:
  BinaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument1, TypePtr argument2);

private:
  String typeName;
  std::map<std::pair<TypePtr, TypePtr>, TypePtr> m;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_CLASS_H_
