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
  LuapeBatchLearner(LuapeLearnerPtr graphLearner, LuapeProblemPtr problem, size_t maxIterations)
    : graphLearner(graphLearner), problem(problem), maxIterations(maxIterations) {}
  LuapeBatchLearner() {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeInferenceClass;}

  const LuapeProblemPtr& getProblem() const
    {return problem;}

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const LuapeInferencePtr& function = f.staticCast<LuapeInference>();
    if (!graphLearner->initialize(context, problem, function))
      return false;
      
    graphLearner->setExamples(context, true, trainingData);
    graphLearner->setExamples(context, false, validationData);
    
    for (size_t i = 0; i < maxIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i);
      graphLearner->doLearningIteration(context);
      context.leaveScope();
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    }
    return true;
  }

protected:
  friend class LuapeBatchLearnerClass;

  LuapeLearnerPtr graphLearner;
  LuapeProblemPtr problem;
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;


#if 0

class BoostingLuapeLearner;
typedef ReferenceCountedObjectPtr<BoostingLuapeLearner> BoostingLuapeLearnerPtr;

class LuapeWeakLearner : public Object
{
public:
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeInferencePtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeWeakLearner> LuapeWeakLearnerPtr;

extern LuapeWeakLearnerPtr singleStumpWeakLearner();
extern LuapeWeakLearnerPtr productWeakLearner(LuapeWeakLearnerPtr baseLearner, size_t numBaseClassifiers);
extern LuapeWeakLearnerPtr luapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps);

extern LuapeWeakLearnerPtr combinedStumpWeakLearner();

class BoostingLuapeLearner : public LuapeBatchLearner
{
public:
  BoostingLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
  BoostingLuapeLearner();

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const = 0;

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeInferencePtr& function, const std::vector<PairPtr>& examples) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const;

  // the absolute value of this quantity should be maximized
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeInferencePtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;
  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class BoostingLuapeLearnerClass;

  LuapeWeakLearnerPtr weakLearner;
  size_t maxIterations;

  void addExamplesToGraph(bool areTrainingSamples, const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph) const;
  double updateWeights(const LuapeInferencePtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const;
  void updatePredictions(const LuapeInferencePtr& function, VectorPtr predictions, const BooleanVectorPtr& weakPredictions, const Variable& vote) const;
};

extern BatchLearnerPtr adaBoostLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
extern BatchLearnerPtr adaBoostMHLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
