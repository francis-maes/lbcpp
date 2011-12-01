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
# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class LuapeBatchLearner : public BatchLearner
{
public:
  LuapeBatchLearner(LuapeLearnerPtr learner, size_t maxIterations)
    : learner(learner), maxIterations(maxIterations) {}
  LuapeBatchLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeInferenceClass;}


  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeInferencePtr& function = f.staticCast<LuapeInference>();

    LuapeLearnerPtr learner = this->learner->cloneAndCast<LuapeLearner>(); // avoid cycle between LuapeInference -> LuapeBatchLearner -> LuapeLearner -> LuapeInference

    if (!learner->initialize(context, function))
      return false;

    learner->setExamples(context, true, trainingData);
    if (validationData.size())
      learner->setExamples(context, false, validationData);
    
    LuapeGraphUniversePtr universe = function->getUniverse();
    for (size_t i = 0; i < maxIterations; ++i)
    {
      //Object::displayObjectAllocationInfo(std::cerr);
      context.informationCallback(T("Train cache size: ") + String((int)learner->getTrainingSamples()->getSize()) +
                                  T("Validation cache size: ") + String((int)learner->getValidationSamples()->getSize()));

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      learner->doLearningIteration(context);
      context.leaveScope();

      //if ((i+1) % 100 == 0)
      //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    }
    //Object::displayObjectAllocationInfo(std::cerr);
    //context.resultCallback("votes", function->getVotes());
    return true;
  }

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr learner;
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
