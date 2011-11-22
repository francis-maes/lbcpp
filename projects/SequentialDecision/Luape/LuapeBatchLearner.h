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
  LuapeBatchLearner(LuapeLearnerPtr learner, LuapeProblemPtr problem, size_t maxIterations)
    : learner(learner), problem(problem), maxIterations(maxIterations) {}
  LuapeBatchLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeInferenceClass;}

  const LuapeProblemPtr& getProblem() const
    {return problem;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeInferencePtr& function = f.staticCast<LuapeInference>();

    LuapeLearnerPtr learner = this->learner->cloneAndCast<LuapeLearner>(); // avoid cycle between LuapeInference -> LuapeBatchLearner -> LuapeLearner -> LuapeInference

    if (!learner->initialize(context, problem, function))
      return false;

    learner->setExamples(context, true, trainingData);
    if (validationData.size())
      learner->setExamples(context, false, validationData);
    
    LuapeGraphUniversePtr universe = learner->getGraph()->getUniverse();
    for (size_t i = 0; i < maxIterations; ++i)
    {
      //Object::displayObjectAllocationInfo(std::cerr);
      universe->displayCacheInformation(context);

      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      learner->doLearningIteration(context);
      context.leaveScope();
      //if ((i+1) % 100 == 0)
      //  context.informationCallback(T("Graph: ") + learner->getGraph()->toShortString());
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    }
    //Object::displayObjectAllocationInfo(std::cerr);
    universe->displayCacheInformation(context);
    return true;
  }

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr learner;
  LuapeProblemPtr problem;
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;


#if 0

class BoostingLuapeLearner;
typedef ReferenceCountedObjectPtr<BoostingLuapeLearner> BoostingLuapeLearnerPtr;

class BoostingWeakLearner : public Object
{
public:
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
};

typedef ReferenceCountedObjectPtr<BoostingWeakLearner> BoostingWeakLearnerPtr;

extern BoostingWeakLearnerPtr singleStumpWeakLearner();
extern BoostingWeakLearnerPtr productWeakLearner(BoostingWeakLearnerPtr baseLearner, size_t numBaseClassifiers);
extern BoostingWeakLearnerPtr luapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps);

extern BoostingWeakLearnerPtr combinedStumpWeakLearner();

class BoostingLuapeLearner : public LuapeBatchLearner
{
public:
  BoostingLuapeLearner(LuapeProblemPtr problem, BoostingWeakLearnerPtr weakLearner, size_t maxIterations);
  BoostingLuapeLearner();

  virtual BoostingWeakObjectivePtr createWeakObjective() const = 0;

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const;

  // the absolute value of this quantity should be maximized
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;
  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class BoostingLuapeLearnerClass;

  BoostingWeakLearnerPtr weakLearner;
  size_t maxIterations;

  void addExamplesToGraph(bool areTrainingSamples, const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph) const;
  double updateWeights(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const;
  void updatePredictions(const LuapeInferencePtr& function, VectorPtr predictions, const BooleanVectorPtr& weakPredictions, const Variable& vote) const;
};

extern BatchLearnerPtr adaBoostLuapeLearner(LuapeProblemPtr problem, BoostingWeakLearnerPtr weakLearner, size_t maxIterations);
extern BatchLearnerPtr adaBoostMHLuapeLearner(LuapeProblemPtr problem, BoostingWeakLearnerPtr weakLearner, size_t maxIterations);
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
