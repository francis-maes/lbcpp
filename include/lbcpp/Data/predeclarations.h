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

# include "../Core/predeclarations.h"

namespace lbcpp
{

class Stream;
typedef ReferenceCountedObjectPtr<Stream> StreamPtr;

class SymmetricMatrix;
typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

class RandomGenerator;
typedef ReferenceCountedObjectPtr<RandomGenerator> RandomGeneratorPtr;

class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;

class SparseDoubleVector;
typedef ReferenceCountedObjectPtr<SparseDoubleVector> SparseDoubleVectorPtr;

class DenseDoubleVector;
typedef ReferenceCountedObjectPtr<DenseDoubleVector> DenseDoubleVectorPtr;

class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PREDECLARATIONS_H_
