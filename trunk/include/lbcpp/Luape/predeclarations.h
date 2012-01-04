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

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeInputNode;
typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeConstantNode;
typedef ReferenceCountedObjectPtr<LuapeConstantNode> LuapeConstantNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeTestNode;
typedef ReferenceCountedObjectPtr<LuapeTestNode> LuapeTestNodePtr;

class LuapeVectorSumNode;
typedef ReferenceCountedObjectPtr<LuapeVectorSumNode> LuapeVectorSumNodePtr;

class LuapeCreateSparseVectorNode;
typedef ReferenceCountedObjectPtr<LuapeCreateSparseVectorNode> LuapeCreateSparseVectorNodePtr;

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

class IterativeLearner;
typedef ReferenceCountedObjectPtr<IterativeLearner> IterativeLearnerPtr;

class BoostingLearner;
typedef ReferenceCountedObjectPtr<BoostingLearner> BoostingLearnerPtr;

class LearningObjective;
typedef ReferenceCountedObjectPtr<LearningObjective> LearningObjectivePtr;

class LuapeGraphBuilderTypeState;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeState> LuapeGraphBuilderTypeStatePtr;

class LuapeGraphBuilderTypeSearchSpace;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeSearchSpace> LuapeGraphBuilderTypeSearchSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PREDECLARATIONS_H_
