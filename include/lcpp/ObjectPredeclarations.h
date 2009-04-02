/*-----------------------------------------.---------------------------------.
| Filename: ObjectPredeclarations.h        | Objects Predeclarations         |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 00:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LCPP_OBJECT_PREDECLARATIONS_H_
# define LCPP_OBJECT_PREDECLARATIONS_H_

# include "Object.h"

namespace lcpp
{

// tools
class ScalarRandomVariableStatistics;
typedef ReferenceCountedObjectPtr<ScalarRandomVariableStatistics> ScalarRandomVariableStatisticsPtr;
class IterationFunction;
typedef ReferenceCountedObjectPtr<IterationFunction> IterationFunctionPtr;
class FeatureDictionary;
typedef ReferenceCountedObjectPtr<FeatureDictionary> FeatureDictionaryPtr;

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
class OptimizerStoppingCriterion;
typedef ReferenceCountedObjectPtr<OptimizerStoppingCriterion> OptimizerStoppingCriterionPtr;
class ScalarOptimizer;
typedef ReferenceCountedObjectPtr<ScalarOptimizer> ScalarOptimizerPtr;
class VectorOptimizer;
typedef ReferenceCountedObjectPtr<VectorOptimizer> VectorOptimizerPtr;

// learning machines
class GradientBasedLearner;
typedef ReferenceCountedObjectPtr<GradientBasedLearner> GradientBasedLearnerPtr;
class Classifier;
typedef ReferenceCountedObjectPtr<Classifier> ClassifierPtr;
class BinaryClassifier;
typedef ReferenceCountedObjectPtr<BinaryClassifier> BinaryClassifierPtr;
class GeneralizedClassifier;
typedef ReferenceCountedObjectPtr<GeneralizedClassifier> GeneralizedClassifierPtr;
class Regressor;
typedef ReferenceCountedObjectPtr<Regressor> RegressorPtr;
class Ranker;
typedef ReferenceCountedObjectPtr<Ranker> RankerPtr;

// gradient based learning machines
class GradientBasedRegressor;
typedef ReferenceCountedObjectPtr<GradientBasedRegressor> GradientBasedRegressorPtr;
class GradientBasedBinaryClassifier;
typedef ReferenceCountedObjectPtr<GradientBasedBinaryClassifier> GradientBasedBinaryClassifierPtr;
class GradientBasedClassifier;
typedef ReferenceCountedObjectPtr<GradientBasedClassifier> GradientBasedClassifierPtr;
class GradientBasedGeneralizedClassifier;
typedef ReferenceCountedObjectPtr<GradientBasedGeneralizedClassifier> GradientBasedGeneralizedClassifierPtr;
class GradientBasedRanker;
typedef ReferenceCountedObjectPtr<GradientBasedRanker> GradientBasedRankerPtr;


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

}; /* namespace lcpp */

#endif // !LCPP_OBJECT_PREDECLARATIONS_H_
