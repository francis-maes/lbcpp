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

#ifndef LBCPP_DATA_PREDECLARATIONS_H_
# define LBCPP_DATA_PREDECLARATIONS_H_

# include "ReferenceCountedObject.h"

namespace lbcpp
{
class Variable;

// tools
class ScalarVariableStatistics;
typedef ReferenceCountedObjectPtr<ScalarVariableStatistics> ScalarVariableStatisticsPtr;
class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;
class StoppingCriterion;
typedef ReferenceCountedObjectPtr<StoppingCriterion> StoppingCriterionPtr;

// new
class Object;
typedef ReferenceCountedObjectPtr<Object> ObjectPtr;
class Type;
typedef ReferenceCountedObjectPtr<Type> TypePtr;
class Class;
typedef ReferenceCountedObjectPtr<Class> ClassPtr;
class Stream;
typedef ReferenceCountedObjectPtr<Stream> StreamPtr;
class Container;
typedef ReferenceCountedObjectPtr<Container> ContainerPtr;
class Vector;
typedef ReferenceCountedObjectPtr<Vector> VectorPtr;
class SymmetricMatrix;
typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;
class Consumer;
typedef ReferenceCountedObjectPtr<Consumer> ConsumerPtr;

// graph
class ObjectGraph;
typedef ReferenceCountedObjectPtr<ObjectGraph> ObjectGraphPtr;

// tables
class TableHeader;
typedef ReferenceCountedObjectPtr<TableHeader> TableHeaderPtr;
class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PREDECLARATIONS_H_
