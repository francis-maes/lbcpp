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

namespace lbcpp
{

extern ClassPtr variableSignatureClass;

class Signature : public NameableObject
{
public:
  Signature(const string& name = string::empty, const string& shortName = string::empty, const string& description = string::empty)
    : NameableObject(name), shortName(shortName), description(description) {}

  const string& getShortName() const
    {return shortName;}

  void setShortName(const string& shortName)
    {this->shortName = shortName;}

  const string& getDescription() const
    {return description;}

  void setDescription(const string& description)
    {this->description = description;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SignatureClass;

  string shortName;
  string description;
};

class VariableSignature : public Signature
{
public:
  VariableSignature(ClassPtr type,
                    const string& name,
                    const string& shortName = string::empty,
                    const string& description = string::empty)
    : Signature(name, shortName, description), type(type) {}
  VariableSignature(const VariableSignature& other)
    : Signature(other.name, other.shortName, other.description) {}
  VariableSignature() {}

  const ClassPtr& getType() const
    {return type;}

  void setType(ClassPtr t)
    {type = t;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class VariableSignatureClass;

  ClassPtr type;
};

class FunctionSignature : public Signature
{
public:
  FunctionSignature(const string& name,
                        const string& shortName = string::empty,
                        const string& description = string::empty,
                        bool isStatic = false)
    : Signature(name, shortName, description), staticFunction(isStatic) {}
  FunctionSignature() {}

  bool isStatic() const
    {return staticFunction;}

protected:
  friend class FunctionSignatureClass;

  bool staticFunction;
};

typedef ReferenceCountedObjectPtr<FunctionSignature> FunctionSignaturePtr;

class LuaFunctionSignature : public FunctionSignature
{
public:
  LuaFunctionSignature(LuaCFunction function,
                        const string& name,
                        const string& shortName = string::empty,
                        const string& description = string::empty,
                        bool isStatic = false)
    : FunctionSignature(name, shortName, description, isStatic), function(function) {}

  LuaFunctionSignature() : function(NULL) {}

  LuaCFunction getFunction() const
    {return function;}

  void setFunction(LuaCFunction function)
    {this->function = function;}

protected:
  friend class LuaFunctionSignatureClass;

  LuaCFunction function;
};

typedef ReferenceCountedObjectPtr<LuaFunctionSignature> LuaFunctionSignaturePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_SIGNATURE_H_
