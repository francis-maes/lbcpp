/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2011 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: Class.h                        | Class introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_CLASS_H_
# define LBCPP_CORE_CLASS_H_

# include "Type.h"

namespace lbcpp
{

class Class : public Type
{
public:
  Class(const String& name, TypePtr baseClass)
    : Type(name, baseClass) {}
  Class(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass)
    : Type(templateType, templateArguments, baseClass) {}
  Class() {}

  virtual bool isAbstract() const
    {return false;}

  virtual String toString() const;
  virtual ClassPtr getClass() const;

  virtual bool isMissingValue(const VariableValue& value) const
    {return !value.getObject();}
    
  virtual VariableValue getMissingValue() const
    {return VariableValue();}

  virtual Variable createFromString(ExecutionContext& context, const String& value) const;
  virtual Variable createFromXml(XmlImporter& importer) const;
  virtual void saveToXml(XmlExporter& exporter, const VariableValue& value) const;

  virtual void destroy(VariableValue& value) const
    {value.clearObject();}

  virtual void copy(VariableValue& dest, const VariableValue& source) const
    {dest.setObject(source.getObjectPointer());}

  virtual String toString(const VariableValue& value) const
    {return value.getObject()->toString();}
  virtual String toShortString(const VariableValue& value) const
    {return value.getObject()->toShortString();}

  virtual int compare(const VariableValue& value1, const VariableValue& value2) const;

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr pairClass(TypePtr firstClass, TypePtr secondClass);

extern ClassPtr typeClass;
extern ClassPtr enumerationClass;
extern ClassPtr classClass;

class DefaultClass : public Class
{
public:
  DefaultClass(const String& name, TypePtr baseClass = objectClass);
  DefaultClass(const String& name, const String& baseClass);
  DefaultClass(TemplateTypePtr templateType, const std::vector<TypePtr>& templateArguments, TypePtr baseClass);
  DefaultClass() : abstractClass(false) {}

  virtual ClassPtr getClass() const;

  virtual bool isAbstract() const
    {return abstractClass;}

  virtual void deinitialize();

  /*
  ** Member variables
  */
  virtual size_t getNumMemberVariables() const;
  virtual VariableSignaturePtr getMemberVariable(size_t index) const;
  bool isMemberVariableGenerated(size_t index) const;

  virtual int findMemberVariable(const String& name) const;

  size_t findOrAddMemberVariable(ExecutionContext& context, const String& name, TypePtr type);

  void reserveMemberVariables(size_t count)
    {variables.reserve(count);}

  size_t addMemberVariable(ExecutionContext& context, TypePtr type, const String& name, const String& shortName = String::empty, const String& description = String::empty, bool isGenerated = false);
  size_t addMemberVariable(ExecutionContext& context, const String& typeName, const String& name, const String& shortName = String::empty, const String& description = String::empty, bool isGenerated = false);
  virtual size_t addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature);

  /*
  ** Member functions
  */
  virtual size_t getNumMemberFunctions() const;
  virtual FunctionSignaturePtr getMemberFunction(size_t index) const;
  virtual int findMemberFunction(const String& name) const;

  size_t addMemberFunction(ExecutionContext& context, LuaFunction function, const String& name, const String& shortName = String::empty, const String& description = String::empty, bool isStatic = false);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DefaultClassClass;

  std::vector<VariableSignaturePtr> variables;
  std::map<String, size_t> variablesMap;

  std::vector<FunctionSignaturePtr> functions;
  std::map<String, size_t> functionsMap;

  bool abstractClass;
};

typedef ReferenceCountedObjectPtr<DefaultClass> DefaultClassPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_CLASS_H_
