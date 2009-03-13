/*-----------------------------------------.---------------------------------.
| Filename: ObjectPredeclarations.h        | Objects Predeclarations         |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 00:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_OBJECT_PREDECLARATIONS_H_
# define CRALGO_OBJECT_PREDECLARATIONS_H_

# include "Object.h"

namespace cralgo
{

// tools
class ScalarRandomVariableStatistics;
typedef ReferenceCountedObjectPtr<ScalarRandomVariableStatistics> ScalarRandomVariableStatisticsPtr;
class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;

// feature visitor
class FeatureVisitor;
typedef ReferenceCountedObjectPtr<FeatureVisitor> FeatureVisitorPtr;
class FeatureGenerator;
typedef ReferenceCountedObjectPtr<FeatureGenerator> FeatureGeneratorPtr;

// vectors
class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;
class SparseVector;
typedef ReferenceCountedObjectPtr<SparseVector> SparseVectorPtr;
class DenseVector;
typedef ReferenceCountedObjectPtr<DenseVector> DenseVectorPtr;
class LazyVector;
typedef ReferenceCountedObjectPtr<LazyVector> LazyVectorPtr;

// learning machines
class GradientBasedLearner;
typedef ReferenceCountedObjectPtr<GradientBasedLearner> GradientBasedLearnerPtr;
class Classifier;
typedef ReferenceCountedObjectPtr<Classifier> ClassifierPtr;
class Regressor;
typedef ReferenceCountedObjectPtr<Regressor> RegressorPtr;
class Ranker;
typedef ReferenceCountedObjectPtr<Ranker> RankerPtr;

// choose functions
class ChooseFunction;
typedef ReferenceCountedObjectPtr<ChooseFunction> ChooseFunctionPtr;
class StateValueFunction;
typedef ReferenceCountedObjectPtr<StateValueFunction> StateValueFunctionPtr;
class ActionValueFunction;
typedef ReferenceCountedObjectPtr<ActionValueFunction> ActionValueFunctionPtr;
class StateFeaturesFunction;
typedef ReferenceCountedObjectPtr<StateFeaturesFunction> StateFeaturesFunctionPtr;
class ActionFeaturesFunction;
typedef ReferenceCountedObjectPtr<ActionFeaturesFunction> ActionFeaturesFunctionPtr;
class StateDescriptionFunction;
typedef ReferenceCountedObjectPtr<StateDescriptionFunction> StateDescriptionFunctionPtr;
class ActionDescriptionFunction;
typedef ReferenceCountedObjectPtr<ActionDescriptionFunction> ActionDescriptionFunctionPtr;

// cralgorithms
class Variable;
typedef ReferenceCountedObjectPtr<Variable> VariablePtr;
class VariableIterator;
typedef ReferenceCountedObjectPtr<VariableIterator> VariableIteratorPtr;
class Choose;
typedef ReferenceCountedObjectPtr<Choose> ChoosePtr;
class CRAlgorithmScope;
typedef ReferenceCountedObjectPtr<CRAlgorithmScope> CRAlgorithmScopePtr;
class CRAlgorithm;
typedef ReferenceCountedObjectPtr<CRAlgorithm> CRAlgorithmPtr;

// policies
class Policy;
typedef ReferenceCountedObjectPtr<Policy> PolicyPtr;

}; /* namespace cralgo */

#endif // !CRALGO_OBJECT_PREDECLARATIONS_H_
