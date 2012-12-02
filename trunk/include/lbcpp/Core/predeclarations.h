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
| Filename: predeclarations.h              | Data Predeclarations            |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 00:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_PREDECLARATIONS_H_
# define LBCPP_CORE_PREDECLARATIONS_H_

# include "ReferenceCountedObject.h"

struct lua_State;
typedef struct lua_State lua_State;
typedef int (*lua_CFunction)(lua_State* L);
typedef struct luaL_Reg luaL_Reg;

namespace lbcpp
{

typedef int (*LuaCFunction)(lua_State* L);

struct ApplicationContext;

class XmlExporter;
class XmlImporter;

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;

class Boolean;
typedef ReferenceCountedObjectPtr<Boolean> BooleanPtr;

class Integer;
typedef ReferenceCountedObjectPtr<Integer> IntegerPtr;

class EnumValue;
typedef ReferenceCountedObjectPtr<EnumValue> EnumValuePtr;

class Double;
typedef ReferenceCountedObjectPtr<Double> DoublePtr;

class String;
typedef ReferenceCountedObjectPtr<String> StringPtr;

class File;
typedef ReferenceCountedObjectPtr<File> FilePtr;

class Directory;
typedef ReferenceCountedObjectPtr<Directory> DirectoryPtr;

class VariableSignature;
typedef ReferenceCountedObjectPtr<VariableSignature> VariableSignaturePtr;

class Class;
typedef ReferenceCountedObjectPtr<Class> ClassPtr;

class Enumeration;
typedef ReferenceCountedObjectPtr<Enumeration> EnumerationPtr;

class DefaultEnumeration;
typedef ReferenceCountedObjectPtr<DefaultEnumeration> DefaultEnumerationPtr;

class TemplateClass;
typedef ReferenceCountedObjectPtr<TemplateClass> TemplateClassPtr;

class Library;
typedef ReferenceCountedObjectPtr<Library> LibraryPtr;

class Pair;
typedef ReferenceCountedObjectPtr<Pair> PairPtr;

class Vector;
typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

class ExecutionContext;
typedef ReferenceCountedObjectPtr<ExecutionContext> ExecutionContextPtr;

extern ExecutionContext& defaultExecutionContext();

class LuaState;
typedef ReferenceCountedObjectPtr<LuaState> LuaStatePtr;

class Loader;
typedef ReferenceCountedObjectPtr<Loader> LoaderPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_PREDECLARATIONS_H_
