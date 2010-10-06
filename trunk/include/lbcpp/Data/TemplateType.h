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
| Filename: TemplateType.h                 | Parameterized Type Generator    |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2010 17:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_TEMPLATE_TYPE_H_
# define LBCPP_DATA_TEMPLATE_TYPE_H_

# include "Type.h"

namespace lbcpp
{

class TemplateType : public NameableObject
{
public:
  TemplateType(const String& name)
    : NameableObject(name), initialized(false) {}
  TemplateType() : initialized(false) {}

  static bool isInstanciatedTypeName(const String& name);
  static bool parseInstanciatedTypeName(const String& typeName, String& templateName, std::vector<String>& arguments, MessageCallback& callback);
  static bool parseInstanciatedTypeName(const String& typeName, String& templateName, std::vector<TypePtr>& templateArguments, MessageCallback& callback);
  static String makeInstanciatedTypeName(const String& name, const std::vector<TypePtr>& templateArguments);

  String makeTypeName(const std::vector<TypePtr>& arguments) const
    {return makeInstanciatedTypeName(getName(), arguments);}

  /*
  ** Initialization
  */
  bool isInitialized() const
    {return initialized;}

  virtual bool initialize(MessageCallback& callback)
    {return (initialized = true);}

  /*
  ** Parameters
  */
  virtual size_t getNumParameters() const = 0;
  virtual String getParameterName(size_t index) const = 0;
  virtual TypePtr getParameterBaseType(size_t index) const = 0;

  virtual TypePtr instantiate(const std::vector<TypePtr>& arguments, MessageCallback& callback) const = 0;
  virtual TypePtr instantiateTypeName(const String& typeNameExpr, const std::vector<TypePtr>& arguments, MessageCallback& callback) const = 0;

  juce_UseDebuggingNewOperator

protected:
  bool initialized;
};

class DefaultTemplateType : public TemplateType
{
public:
  DefaultTemplateType(const String& name, const String& baseTypeExpr);
  DefaultTemplateType() {}

  virtual size_t getNumParameters() const;
  virtual String getParameterName(size_t index) const;
  virtual TypePtr getParameterBaseType(size_t index) const;

  int findParameter(const String& name) const;

  virtual TypePtr instantiate(const std::vector<TypePtr>& arguments, TypePtr baseType, MessageCallback& callback) const = 0;
  virtual TypePtr instantiate(const std::vector<TypePtr>& arguments, MessageCallback& callback) const;

  virtual TypePtr instantiateTypeName(const String& typeNameExpr, const std::vector<TypePtr>& arguments, MessageCallback& callback) const;

  juce_UseDebuggingNewOperator

protected:

  String baseTypeExpr;
  std::vector<std::pair<String, TypePtr> > parameters;

  void addParameter(const String& name, TypePtr baseType = anyType());
  void addParameter(const String& name, const String& type);
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_TEMPLATE_TYPE_H_
