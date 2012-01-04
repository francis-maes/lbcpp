/*-----------------------------------------.---------------------------------.
| Filename: LuapeBatchLearner.h            | Luape Batch Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_H_
# define LBCPP_LUAPE_BATCH_LEARNER_H_

# include "LuapeLearner.h"
# include "LuapeCache.h"
# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class LuapeBatchLearner : public BatchLearner
{
public:
  LuapeBatchLearner(LuapeLearnerPtr learner)
    : learner(learner) {}
  LuapeBatchLearner() {}
 
  virtual TypePtr getRequiredFunctionType() const
    {return luapeInferenceClass;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeInferencePtr& problem = f.staticCast<LuapeInference>();
    problem->setSamples(context, trainingData, validationData);
    
    // create initial node
    LuapeNodePtr node = problem->getRootNode();
    if (!node)
      node = learner->createInitialNode(context, problem);
    if (node)
      problem->setRootNode(context, node);

    // learn
    node = learner->learn(context, node, problem, problem->getTrainingCache()->getAllIndices());
    if (!node)
      return false;
    problem->setRootNode(context, node);
    return true;
  }

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr learner;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
