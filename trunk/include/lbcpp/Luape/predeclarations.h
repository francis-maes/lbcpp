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

class LuapeSampleVector;
typedef ReferenceCountedObjectPtr<LuapeSampleVector> LuapeSampleVectorPtr;

class LuapeInstanceCache;
typedef ReferenceCountedObjectPtr<LuapeInstanceCache> LuapeInstanceCachePtr;

class LuapeSamplesCache;
typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

class ExpressionDomain;
typedef ReferenceCountedObjectPtr<ExpressionDomain> ExpressionDomainPtr;

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

class ExpressionRPNSequence;
typedef ReferenceCountedObjectPtr<ExpressionRPNSequence> ExpressionRPNSequencePtr;

class ExpressionRPNTypeState;
typedef ReferenceCountedObjectPtr<ExpressionRPNTypeState> ExpressionRPNTypeStatePtr;

class ExpressionRPNTypeSpace;
typedef ReferenceCountedObjectPtr<ExpressionRPNTypeSpace> ExpressionRPNTypeSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PREDECLARATIONS_H_
