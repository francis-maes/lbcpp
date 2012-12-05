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
| Filename: TemplateClass.h                | Parameterized Class Generator   |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_TEMPLATE_CLASS_H_
# define LBCPP_CORE_TEMPLATE_CLASS_H_

# include "Class.h"

namespace lbcpp
{

extern ClassPtr templateClassClass;

class TemplateClass : public NameableObject
{
public:
  TemplateClass(const string& name)
    : NameableObject(name), initialized(false) {}
  TemplateClass() : initialized(false) {}

  static bool isInstanciatedTypeName(const string& name);
  static bool parseInstanciatedTypeName(ExecutionContext& context, const string& typeName, string& templateName, std::vector<string>& arguments);
  static bool parseInstanciatedTypeName(ExecutionContext& context, const string& typeName, string& templateName, std::vector<ClassPtr>& templateArguments);
  static string makeInstanciatedTypeName(const string& name, const std::vector<ClassPtr>& templateArguments);

  string makeTypeName(const std::vector<ClassPtr>& arguments) const
    {return makeInstanciatedTypeName(getName(), arguments);}

  /*
  ** Initialization
  */
  bool isInitialized() const
    {return initialized;}

  virtual bool initialize(ExecutionContext& context)
    {return (initialized = true);}

  /*
  ** Parameters
  */
  virtual size_t getNumParameters() const = 0;
  virtual string getParameterName(size_t index) const = 0;
  virtual ClassPtr getParameterBaseType(size_t index) const = 0;

  virtual ClassPtr instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments) const = 0;
  virtual ClassPtr instantiateTypeName(ExecutionContext& context, const string& typeNameExpr, const std::vector<ClassPtr>& arguments) const = 0;

  /*
  ** Object
  */
  virtual ClassPtr getClass() const
    {return templateClassClass;}

  lbcpp_UseDebuggingNewOperator

protected:
  bool initialized;
};

class DefaultTemplateClass : public TemplateClass
{
public:
  DefaultTemplateClass(const string& name, const string& baseTypeExpr);
  DefaultTemplateClass() {}

  virtual size_t getNumParameters() const;
  virtual string getParameterName(size_t index) const;
  virtual ClassPtr getParameterBaseType(size_t index) const;

  int findParameter(const string& name) const;

  virtual ClassPtr instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments, ClassPtr baseType) const = 0;
  virtual ClassPtr instantiate(ExecutionContext& context, const std::vector<ClassPtr>& arguments) const;

  virtual ClassPtr instantiateTypeName(ExecutionContext& context, const string& typeNameExpr, const std::vector<ClassPtr>& arguments) const;

  lbcpp_UseDebuggingNewOperator

protected:
  string baseTypeExpr;
  std::vector<std::pair<string, ClassPtr> > parameters;

  void addParameter(const string& name, ClassPtr baseType = objectClass);
  void addParameter(ExecutionContext& context, const string& name, const string& type);
  bool inheritsFrom(ExecutionContext& context, const std::vector<ClassPtr>& arguments, const string& parameterName, const string& baseType) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_TEMPLATE_CLASS_H_
