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

namespace lbcpp
{

class Variable;
class XmlExporter;
class XmlImporter;

class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;

class Type;
typedef NativePtr<Type> TypePtr;

class Class;
typedef NativePtr<Class> ClassPtr;

class Enumeration;
typedef NativePtr<Enumeration> EnumerationPtr;

class TemplateType;
typedef ReferenceCountedObjectPtr<TemplateType> TemplateTypePtr;

class Pair;
typedef ReferenceCountedObjectPtr<Pair> PairPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_PREDECLARATIONS_H_
