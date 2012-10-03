/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Luape Predeclarations           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 10:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PREDECLARATIONS_H_
# define LBCPP_LUAPE_PREDECLARATIONS_H_

# include "../Core/predeclarations.h"

namespace lbcpp
{

class Expression;
typedef ReferenceCountedObjectPtr<Expression> ExpressionPtr;

class VariableExpression;
typedef ReferenceCountedObjectPtr<VariableExpression> VariableExpressionPtr;

class ConstantExpression;
typedef ReferenceCountedObjectPtr<ConstantExpression> ConstantExpressionPtr;

class FunctionExpression;
typedef ReferenceCountedObjectPtr<FunctionExpression> FunctionExpressionPtr;

class TestExpression;
typedef ReferenceCountedObjectPtr<TestExpression> TestExpressionPtr;

class VectorSumExpression;
typedef ReferenceCountedObjectPtr<VectorSumExpression> VectorSumExpressionPtr;

class CreateSparseVectorExpression;
typedef ReferenceCountedObjectPtr<CreateSparseVectorExpression> CreateSparseVectorExpressionPtr;

class LuapeSampleVector;
typedef ReferenceCountedObjectPtr<LuapeSampleVector> LuapeSampleVectorPtr;

class LuapeInstanceCache;
typedef ReferenceCountedObjectPtr<LuapeInstanceCache> LuapeInstanceCachePtr;

class LuapeSamplesCache;
typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

class LuapeInference;
typedef ReferenceCountedObjectPtr<LuapeInference> LuapeInferencePtr;

class LuapeLearner;
typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class DecoratorLearner;
typedef ReferenceCountedObjectPtr<DecoratorLearner> DecoratorLearnerPtr;

class NodeBuilderBasedLearner;
typedef ReferenceCountedObjectPtr<NodeBuilderBasedLearner> NodeBuilderBasedLearnerPtr;

class IterativeLearner;
typedef ReferenceCountedObjectPtr<IterativeLearner> IterativeLearnerPtr;

class BoostingLearner;
typedef ReferenceCountedObjectPtr<BoostingLearner> BoostingLearnerPtr;

class LearningObjective;
typedef ReferenceCountedObjectPtr<LearningObjective> LearningObjectivePtr;

class LuapeRPNSequence;
typedef ReferenceCountedObjectPtr<LuapeRPNSequence> LuapeRPNSequencePtr;

class LuapeGraphBuilderTypeState;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeState> LuapeGraphBuilderTypeStatePtr;

class LuapeGraphBuilderTypeSearchSpace;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeSearchSpace> LuapeGraphBuilderTypeSearchSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PREDECLARATIONS_H_
