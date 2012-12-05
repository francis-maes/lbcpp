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
| Filename: DefaultClass.h                 | Class introspection             |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_CORE_DEFAULT_CLASS_H_
# define OIL_CORE_DEFAULT_CLASS_H_

# include "Class.h"

namespace lbcpp
{

class DefaultClass : public Class
{
public:
  DefaultClass(const string& name, ClassPtr baseClass = objectClass);
  DefaultClass(const string& name, const string& baseClass);
  DefaultClass(TemplateClassPtr templateType, const std::vector<ClassPtr>& templateArguments, ClassPtr baseClass);
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

  virtual int findMemberVariable(const string& name) const;

  size_t findOrAddMemberVariable(ExecutionContext& context, const string& name, ClassPtr type);

  void reserveMemberVariables(size_t count)
    {variables.reserve(count);}

  size_t addMemberVariable(ExecutionContext& context, ClassPtr type, const string& name, const string& shortName = string::empty, const string& description = string::empty, bool isGenerated = false);
  size_t addMemberVariable(ExecutionContext& context, const string& typeName, const string& name, const string& shortName = string::empty, const string& description = string::empty, bool isGenerated = false);
  virtual size_t addMemberVariable(ExecutionContext& context, VariableSignaturePtr signature);

  /*
  ** Member functions
  */
  virtual size_t getNumMemberFunctions() const;
  virtual FunctionSignaturePtr getMemberFunction(size_t index) const;
  virtual int findMemberFunction(const string& name) const;

  size_t addMemberFunction(ExecutionContext& context, LuaCFunction function, const string& name, const string& shortName = string::empty, const string& description = string::empty, bool isStatic = false);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DefaultClassClass;

  std::vector<VariableSignaturePtr> variables;
  std::map<string, size_t> variablesMap;

  std::vector<FunctionSignaturePtr> functions;
  std::map<string, size_t> functionsMap;

  bool abstractClass;
};

typedef ReferenceCountedObjectPtr<DefaultClass> DefaultClassPtr;

}; /* namespace lbcpp */

#endif // !OIL_CORE_DEFAULT_CLASS_H_
