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

#ifndef LBCPP_CORE_TYPE_H_
# define LBCPP_CORE_TYPE_H_

# include "Object.h"
# include "../Execution/ExecutionContext.h"
# include "impl/VariableValue.hpp"

namespace lbcpp
{

class VariableSignature : public NameableObject
{
public:
  VariableSignature(TypePtr type,
                    const String& name,
                    const String& shortName = String::empty,
                    const String& description = String::empty)
    : NameableObject(name), type(type), shortName(shortName), description(description) {}
  VariableSignature(const VariableSignature& other)
    : NameableObject(other.name), type(other.type), shortName(other.shortName), description(other.description) {}
  VariableSignature() {}

  const TypePtr& getType() const
    {return type;}

  void setType(TypePtr t)
    {type = t;}

  const String& getShortName() const
    {return shortName;}

  void setShortName(const String& shortName)
    {this->shortName = shortName;}

  const String& getDescription() const
    {return description;}

  void setDescription(const String& description)
    {this->description = description;}

protected:
  friend class VariableSignatureClass;

  TypePtr type;
  String shortName;
  String description;
};

class Type : public NameableObject
{
public:
  Type(const String& className, TypePtr baseType);
  Type(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseType);
  Type() {}
  virtual ~Type();

  /*
  ** Initialization
  */
  bool isInitialized() const
    {return initialized;}

  virtual bool initialize(ExecutionContext& context);
  virtual void deinitialize();

  /*
  ** Name
  */
  virtual bool isUnnamedType() const;

  /*
  ** Type operations
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

  size_t getNumTemplateArguments() const
    {return templateArguments.size();}
  
  TypePtr getTemplateArgument(size_t index) const
    {jassert(index < templateArguments.size()); return templateArguments[index];}
  
  /*
  ** Operations
  */
  virtual VariableValue getMissingValue() const;
  virtual bool isMissingValue(const VariableValue& value) const;

  virtual VariableValue create(ExecutionContext& context) const;
  virtual VariableValue createFromString(ExecutionContext& context, const String& value) const;
  virtual VariableValue createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const;
  virtual void copy(VariableValue& dest, const VariableValue& source) const;
  virtual String toString(const VariableValue& value) const;
  virtual String toShortString(const VariableValue& value) const
    {return toString(value);}
  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  /*
  ** Member Variables
  */
  virtual size_t getNumMemberVariables() const;
  virtual VariableSignaturePtr getMemberVariable(size_t index) const;

  TypePtr getMemberVariableType(size_t index) const;
  String getMemberVariableName(size_t index) const;
  String getMemberVariableShortName(size_t index) const;
  String getMemberVariableDescription(size_t index) const;
  VariableSignaturePtr getLastMemberVariable() const;

  virtual int findMemberVariable(const String& name) const;
  virtual Variable getMemberVariableValue(const Object* pthis, size_t index) const;
  virtual void setMemberVariableValue(Object* pthis, size_t index, const Variable& subValue) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  static TypePtr loadUnnamedTypeFromXml(XmlImporter& importer);

  virtual String toString() const
    {return getName();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TypeClass;

  bool initialized;

  TypePtr baseType;
  TemplateTypePtr templateType;
  std::vector<TypePtr> templateArguments;
};

extern TypePtr variableType;

// synonims of variableType
extern TypePtr topLevelType;
extern TypePtr anyType;

extern TypePtr nilType;

extern TypePtr booleanType;
extern TypePtr integerType;
  extern TypePtr positiveIntegerType;
    extern TypePtr variableIndexType;
  extern TypePtr enumValueType;

extern TypePtr doubleType;
  extern TypePtr positiveDoubleType;
    extern TypePtr negativeLogProbabilityType;
  extern TypePtr probabilityType;

extern TypePtr stringType;
  extern TypePtr fileType;

extern TypePtr sumType(TypePtr type1, TypePtr type2);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3);
extern TypePtr sumType(TypePtr type1, TypePtr type2, TypePtr type3, TypePtr type4);
extern TypePtr sumType(const std::vector<TypePtr>& types);

extern ClassPtr objectClass;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TYPE_H_
