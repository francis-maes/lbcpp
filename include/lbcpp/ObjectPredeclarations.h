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
| Filename: ObjectPredeclarations.h        | Objects Predeclarations         |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 00:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PREDECLARATIONS_H_
# define LBCPP_OBJECT_PREDECLARATIONS_H_

# include "Data/Object.h"
# include "Data/Type.h"

namespace lbcpp
{

// reference counted objects
class StringDictionary;
typedef ReferenceCountedObjectPtr<StringDictionary> StringDictionaryPtr;

// tools
class ScalarVariableStatistics;
typedef ReferenceCountedObjectPtr<ScalarVariableStatistics> ScalarVariableStatisticsPtr;
class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;
class FeatureDictionary;
typedef ReferenceCountedObjectPtr<FeatureDictionary> FeatureDictionaryPtr;

// new
class Function;
typedef ReferenceCountedObjectPtr<Function> FunctionPtr;
class Stream;
typedef ReferenceCountedObjectPtr<Stream> StreamPtr;
class Container;
typedef ReferenceCountedObjectPtr<Container> ContainerPtr;
class Vector;
typedef ReferenceCountedObjectPtr<Vector> VectorPtr;
class SymmetricMatrix;
typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

// object stream
class ObjectStream;
typedef ReferenceCountedObjectPtr<ObjectStream> ObjectStreamPtr;
class LearningDataObjectParser;
typedef ReferenceCountedObjectPtr<LearningDataObjectParser> LearningDataObjectParserPtr;

// object containers
class ObjectContainer;
typedef ReferenceCountedObjectPtr<ObjectContainer> ObjectContainerPtr;
class VectorObjectContainer;
typedef ReferenceCountedObjectPtr<VectorObjectContainer> VectorObjectContainerPtr;

// object consumer
class ObjectConsumer;
typedef ReferenceCountedObjectPtr<ObjectConsumer> ObjectConsumerPtr;

// object function
class ObjectFunction;
typedef ReferenceCountedObjectPtr<ObjectFunction> ObjectFunctionPtr;

// graph
class ObjectGraph;
typedef ReferenceCountedObjectPtr<ObjectGraph> ObjectGraphPtr;

// tables
class TableHeader;
typedef ReferenceCountedObjectPtr<TableHeader> TableHeaderPtr;
class Table;
typedef ReferenceCountedObjectPtr<Table> TablePtr;

// feature generators
class FeatureVisitor;
typedef ReferenceCountedObjectPtr<FeatureVisitor> FeatureVisitorPtr;
class FeatureGenerator;
typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

class EditableFeatureGenerator;
typedef ReferenceCountedObjectPtr<EditableFeatureGenerator> EditableFeatureGeneratorPtr;
class WeightedFeatureGenerator;
typedef ReferenceCountedObjectPtr<WeightedFeatureGenerator> WeightedFeatureGeneratorPtr;
class LinearCombinationFeatureGenerator;
typedef ReferenceCountedObjectPtr<LinearCombinationFeatureGenerator> LinearCombinationFeatureGeneratorPtr;
class SubFeatureGenerator;
typedef ReferenceCountedObjectPtr<SubFeatureGenerator> SubFeatureGeneratorPtr;
class CompositeFeatureGenerator;
typedef ReferenceCountedObjectPtr<CompositeFeatureGenerator> CompositeFeatureGeneratorPtr;
class FeatureVector;
typedef ReferenceCountedObjectPtr<FeatureVector> FeatureVectorPtr;
class SparseVector;
typedef ReferenceCountedObjectPtr<SparseVector> SparseVectorPtr;
class DenseVector;
typedef ReferenceCountedObjectPtr<DenseVector> DenseVectorPtr;

// functions
class ScalarFunction;
typedef ReferenceCountedObjectPtr<ScalarFunction> ScalarFunctionPtr;
class ScalarLossFunction;
typedef ReferenceCountedObjectPtr<ScalarLossFunction> ScalarLossFunctionPtr;
class ScalarVectorFunction;
typedef ReferenceCountedObjectPtr<ScalarVectorFunction> ScalarVectorFunctionPtr;
class VectorLossFunction;
typedef ReferenceCountedObjectPtr<VectorLossFunction> VectorLossFunctionPtr;
class ScalarArchitecture;
typedef ReferenceCountedObjectPtr<ScalarArchitecture> ScalarArchitecturePtr;
class VectorArchitecture;
typedef ReferenceCountedObjectPtr<VectorArchitecture> VectorArchitecturePtr;

// optimizer
class StoppingCriterion;
typedef ReferenceCountedObjectPtr<StoppingCriterion> StoppingCriterionPtr;
class ScalarOptimizer;
typedef ReferenceCountedObjectPtr<ScalarOptimizer> ScalarOptimizerPtr;
class VectorOptimizer;
typedef ReferenceCountedObjectPtr<VectorOptimizer> VectorOptimizerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PREDECLARATIONS_H_
