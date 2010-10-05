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
| Filename: Type.h                         | The class interface for         |
| Author  : Francis Maes                   |  introspection                  |
| Started : 24/06/2010 11:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_TYPE_H_
# define LBCPP_DATA_TYPE_H_

# include "Object.h"
# include "impl/VariableValue.hpp"

namespace lbcpp
{

extern void initialize();
extern void deinitialize();

class TemplateType;
typedef ReferenceCountedObjectPtr<TemplateType> TemplateTypePtr;
class Vector;
typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

class Type : public NameableObject
{
public:
  Type(const String& className, TypePtr baseType);
  Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType);
  Type() {}

  static void declare(TypePtr typeInstance);
  static void declare(TemplateTypePtr templateTypeInstance);
  static void finishDeclarations(MessageCallback& callback = MessageCallback::getInstance());
  static bool doTypeExists(const String& typeName);
  static TypePtr get(const String& typeName, MessageCallback& callback = MessageCallback::getInstance());
  static TypePtr get(const String& name, const std::vector<TypePtr>& arguments, MessageCallback& callback = MessageCallback::getInstance());

  /*
  ** Initialization
  */
  bool isInitialized() const
    {return initialized;}

  virtual bool initialize(MessageCallback& callback);
  virtual void deinitialize();

  /*
  ** Base type
  */
  TypePtr getBaseType() const
    {return baseType;}

  void setBaseType(TypePtr baseType)
    {this->baseType = baseType;}

  bool inheritsFrom(TypePtr baseType) const;
  bool canBeCastedTo(TypePtr targetType) const;

  static TypePtr findCommonBaseType(TypePtr type1, TypePtr type2);

  /*
  ** Template Arguments
  */
  TemplateTypePtr getTemplate() const
    {return templateType;}

  const std::vector<TypePtr>& getTemplateArguments() const
    {return templateArguments;}

  TypePtr getTemplateArgument(size_t index) const;
  size_t getNumTemplateArguments() const;

  void setTemplate(TemplateTypePtr type, const std::vector<TypePtr>& arguments)
    {templateType = type; templateArguments = arguments;}

  /*
  ** Operations
  */
  virtual VariableValue getMissingValue() const;
  virtual bool isMissingValue(const VariableValue& value) const;

  virtual VariableValue create() const;
  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const;
  virtual void copy(VariableValue& dest, const VariableValue& source) const;
  virtual String toString(const VariableValue& value) const;
  virtual String toShortString(const VariableValue& value) const
    {return toString(value);}
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  /*
  ** Static Variables
  */
  virtual size_t getObjectNumVariables() const;
  virtual TypePtr getObjectVariableType(size_t index) const;
  virtual String getObjectVariableName(size_t index) const;
  virtual int findObjectVariable(const String& name) const;
  virtual Variable getObjectVariable(const VariableValue& value, size_t index) const;
  virtual void setObjectVariable(const VariableValue& value, size_t index, const Variable& subValue) const;

  /*
  ** Dynamic Variables
  */
  virtual size_t getNumElements(const VariableValue& value) const;
  virtual Variable getElement(const VariableValue& value, size_t index) const;
  virtual String getElementName(const VariableValue& value, size_t index) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const;

  virtual String toString() const
    {return getName();}

  virtual void saveToXml(XmlExporter& exporter) const;

  juce_UseDebuggingNewOperator

protected:
  friend class TypeClass;

  bool initialized;

  TypePtr baseType;
  TemplateTypePtr templateType;
  std::vector<TypePtr> templateArguments;
};

extern TypePtr variableType();

// synonims:
inline TypePtr topLevelType()
  {return variableType();}
inline TypePtr anyType()
  {return variableType();}

extern TypePtr nilType();

extern TypePtr booleanType();
extern TypePtr integerType();
  extern TypePtr positiveIntegerType();
  extern TypePtr enumValueType();

extern TypePtr doubleType();
  extern TypePtr positiveDoubleType();
    extern TypePtr negativeLogProbabilityType();
  extern TypePtr probabilityType();

extern TypePtr stringType();
  extern TypePtr fileType();

extern TypePtr pairType(TypePtr firstClass, TypePtr secondClass);

extern TypePtr sumType(TypePtr type1, TypePtr type2);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3, TypePtr type4);
extern TypePtr sumType(const std::vector<TypePtr>& types);

/*
** Enumeration
*/
class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class Enumeration : public Type
{
public:
  Enumeration(const String& name, const juce::tchar** elements, const String& oneLetterCodes = String::empty);
  Enumeration(const String& name, const String& oneLetterCodes);
  Enumeration(const String& name);

  virtual ClassPtr getClass() const;

  static EnumerationPtr get(const String& className)
    {return checkCast<Enumeration>(T("Enumeration::get"), Type::get(className));}

  virtual VariableValue create() const;
  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;

  virtual VariableValue getMissingValue() const;

  virtual String toString(const VariableValue& value) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual size_t getNumElements(const VariableValue& value) const
    {return 0;}

  // FIXME: names are not very good
  size_t getNumElements() const
    {return elements.size();}

  String getElementName(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  int findElement(const String& name) const;
  // --

  bool hasOneLetterCodes() const;
  juce::tchar getOneLetterCode(size_t index) const;
  String getOneLetterCodes() const;

  juce_UseDebuggingNewOperator

protected:
  void addElement(const String& elementName, const String& oneLetterCode = String::empty, const String& threeLettersCode = String::empty);

private:
  friend class EnumerationClass;

  std::vector<String> elements; // use Vector ?
  String oneLetterCodes;
};

/*
** Class
*/
class Class : public Type
{
public:
  Class(const String& name, TypePtr baseClass)
    : Type(name, baseClass) {}
  Class(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Type(templateType, templateArguments, baseClass) {}
  Class() {}

  virtual String toString() const;

  virtual bool isMissingValue(const VariableValue& value) const
    {return !value.getObject();}
    
  virtual VariableValue getMissingValue() const
    {return VariableValue();}

  virtual VariableValue createFromString(const String& value, MessageCallback& callback) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.clearObject(); dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}
  virtual String toShortString(const VariableValue& value) const
    {return value.getObject()->toShortString();}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  juce_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Class> ClassPtr;

extern ClassPtr objectClass();
extern ClassPtr typeClass();
extern ClassPtr enumerationClass();

class DefaultClass : public Class
{
public:
  DefaultClass(const String& name, TypePtr baseClass = objectClass());
  DefaultClass(const String& name, const String& baseClass);
  DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass);
  DefaultClass() {}

  virtual void deinitialize();

  virtual size_t getObjectNumVariables() const;
  virtual TypePtr getObjectVariableType(size_t index) const;
  virtual String getObjectVariableName(size_t index) const;
  virtual int findObjectVariable(const String& name) const;

  void addVariable(TypePtr type, const String& name);
  void addVariable(const String& typeName, const String& name);

  juce_UseDebuggingNewOperator

protected:
  std::vector< std::pair<TypePtr, String> > variables;
};

typedef ReferenceCountedObjectPtr<DefaultClass> DefaultClassPtr;

/*
** Inheritance check
*/
inline bool checkInheritance(TypePtr type, TypePtr baseType, MessageCallback& callback = MessageCallback::getInstance())
{
  jassert(baseType);
  if (!type || !type->inheritsFrom(baseType))
  {
    callback.errorMessage(T("checkInheritance"), T("Invalid type, Expected ") + baseType->getName().quoted() + T(" found ") + (type ? type->getName().quoted() : T("Nil")));
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
    {return TypePtr(type);}

protected:
  Type* type;
};

class UnaryTemplateTypeCache
{
public:
  UnaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument);

private:
  String typeName;
  CriticalSection lock;
  std::map<Type*, Type*> m;
};

class BinaryTemplateTypeCache
{
public:
  BinaryTemplateTypeCache(const String& typeName)
    : typeName(typeName) {}

  TypePtr operator ()(TypePtr argument1, TypePtr argument2);

private:
  String typeName;
  CriticalSection lock;
  std::map<std::pair<Type*, Type*>, Type*> m;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TYPE_H_
