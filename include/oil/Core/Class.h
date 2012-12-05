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

#ifndef OIL_CORE_TYPE_H_
# define OIL_CORE_TYPE_H_

# include "Signature.h"

namespace lbcpp
{

class Class : public NameableObject
{
public:
  Class(const string& className, ClassPtr baseType);
  Class(TemplateClassPtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseType);
  Class() : namedType(false) {}
  virtual ~Class();

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
  bool isNamedType() const
    {return namedType;}

  const string& getShortName() const
    {return shortName;}
  
  /*
  ** Type operations
  */
  ClassPtr getBaseType() const
    {return baseType;}

  void setBaseType(ClassPtr baseType)
    {this->baseType = baseType;}

  ClassPtr findBaseTypeFromTemplateName(const string& templateName) const;

  bool inheritsFrom(ClassPtr baseType) const;
  bool canBeCastedTo(ClassPtr targetType) const;
  
  static ClassPtr findCommonBaseClass(ClassPtr type1, ClassPtr type2);

  /*
  ** Template Arguments
  */
  TemplateClassPtr getTemplate() const
    {return templateType;}

  const std::vector<ClassPtr>& getTemplateArguments() const
    {return templateArguments;}

  size_t getNumTemplateArguments() const
    {return templateArguments.size();}
  
  ClassPtr getTemplateArgument(size_t index) const
    {jassert(index < templateArguments.size()); return templateArguments[index];}
  
  /*
  ** Object Constructor
  */
  virtual ObjectPtr createObject(ExecutionContext& context) const;

  /*
  ** Flags
  */
  virtual bool isAbstract() const
    {return false;}
  virtual bool isConvertibleToBoolean() const;
  virtual bool isConvertibleToDouble() const;

  /*
  ** Member Variables
  */
  virtual size_t getNumMemberVariables() const;
  virtual VariableSignaturePtr getMemberVariable(size_t index) const;

  ClassPtr getMemberVariableType(size_t index) const;
  string getMemberVariableName(size_t index) const;
  string getMemberVariableShortName(size_t index) const;
  string getMemberVariableDescription(size_t index) const;
  VariableSignaturePtr getLastMemberVariable() const;

  string makeUniqueMemberVariableName(const string& name) const;
  virtual int findMemberVariable(const string& name) const;
  virtual ObjectPtr getMemberVariableValue(const Object* pthis, size_t index) const;
  virtual void setMemberVariableValue(Object* pthis, size_t index, const ObjectPtr& subValue) const;

  /*
  ** Member Functions
  */
  virtual size_t getNumMemberFunctions() const;
  virtual FunctionSignaturePtr getMemberFunction(size_t index) const;
  virtual int findMemberFunction(const string& name) const;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const;

  virtual void saveToXml(XmlExporter& exporter) const;
  virtual string toString() const;
  virtual string toShortString() const
    {return shortName.isNotEmpty() ? shortName : toString();}

  void luaRegister(LuaState& state) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ClassClass;
  friend class ClassManager;
  friend struct TemplateClassCache;

  bool initialized;

  ClassPtr baseType;
  TemplateClassPtr templateType;
  std::vector<ClassPtr> templateArguments;
  bool namedType;
  string shortName;
};

extern ClassPtr classClass;

}; /* namespace lbcpp */

#endif // !OIL_CORE_TYPE_H_
