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
| Filename: Signature.h                    | Variable & Function Signatures  |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 17:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_SIGNATURE_H_
# define LBCPP_CORE_SIGNATURE_H_

# include "Object.h"
# include "../Lua/Lua.h"

namespace lbcpp
{

extern ClassPtr variableSignatureClass;

class Signature : public NameableObject
{
public:
  Signature(const String& name = String::empty, const String& shortName = String::empty, const String& description = String::empty)
    : NameableObject(name), shortName(shortName), description(description) {}

  const String& getShortName() const
    {return shortName;}

  void setShortName(const String& shortName)
    {this->shortName = shortName;}

  const String& getDescription() const
    {return description;}

  void setDescription(const String& description)
    {this->description = description;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SignatureClass;

  String shortName;
  String description;
};

class VariableSignature : public Signature
{
public:
  VariableSignature(TypePtr type,
                    const String& name,
                    const String& shortName = String::empty,
                    const String& description = String::empty)
    : Signature(name, shortName, description), type(type) {}
  VariableSignature(const VariableSignature& other)
    : Signature(other.name, other.shortName, other.description) {}
  VariableSignature() {}

  const TypePtr& getType() const
    {return type;}

  void setType(TypePtr t)
    {type = t;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class VariableSignatureClass;

  TypePtr type;
};

class FunctionSignature : public Signature
{
public:
  FunctionSignature(const String& name,
                        const String& shortName = String::empty,
                        const String& description = String::empty)
    : Signature(name, shortName, description) {}
  FunctionSignature() {}
};

typedef ReferenceCountedObjectPtr<FunctionSignature> FunctionSignaturePtr;

class LuaFunctionSignature : public FunctionSignature
{
public:
  LuaFunctionSignature(LuaFunction function,
                        const String& name,
                        const String& shortName = String::empty,
                        const String& description = String::empty)
    : FunctionSignature(name, shortName, description), function(function) {}
  LuaFunctionSignature() : function(NULL) {}

  LuaFunction getFunction() const
    {return function;}

  void setFunction(LuaFunction function)
    {this->function = function;}

protected:
  friend class LuaFunctionSignatureClass;

  LuaFunction function;
};

typedef ReferenceCountedObjectPtr<LuaFunctionSignature> LuaFunctionSignaturePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_SIGNATURE_H_
