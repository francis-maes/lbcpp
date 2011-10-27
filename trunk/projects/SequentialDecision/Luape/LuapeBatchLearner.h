/*-----------------------------------------.---------------------------------.
| Filename: LuapeBatchLearner.h            | Luape Batch Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BATCH_LEARNER_H_
# define LBCPP_LUAPE_BATCH_LEARNER_H_

# include "LuapeProblem.h"
# include "LuapeFunction.h"
# include <lbcpp/Learning/BatchLearner.h>

namespace lbcpp
{

class BoostingLuapeLearner;
typedef ReferenceCountedObjectPtr<BoostingLuapeLearner> BoostingLuapeLearnerPtr;

/*
** LuapeWeakLearner
*/
class LuapeWeakLearner : public Object
{
public:
  virtual LuapeGraphPtr learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeWeakLearner> LuapeWeakLearnerPtr;

extern LuapeWeakLearnerPtr luapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps);

/*
** Boosting learner
*/
class LuapeBatchLearner : public BatchLearner
{
public:
  LuapeBatchLearner(LuapeProblemPtr problem = LuapeProblemPtr())
    : problem(problem) {}

  virtual TypePtr getRequiredFunctionType() const
    {return luapeFunctionClass;}

  const LuapeProblemPtr& getProblem() const
    {return problem;}

protected:
  friend class LuapeBatchLearnerClass;

  LuapeProblemPtr problem;
};

typedef ReferenceCountedObjectPtr<LuapeBatchLearner> LuapeBatchLearnerPtr;

class BoostingLuapeLearner : public LuapeBatchLearner
{
public:
  BoostingLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
  BoostingLuapeLearner();

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const = 0;

  // the absolute value of this quantity should be maximized
  virtual double computeWeakObjective(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;

  virtual VectorPtr createVoteVector(const LuapeFunctionPtr& function) const = 0;
  virtual Variable computeVote(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double weakObjectiveValue) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class BoostingLuapeLearnerClass;

  LuapeWeakLearnerPtr weakLearner;
  size_t maxIterations;

  void addExamplesToGraph(const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph, VectorPtr& supervisions) const;
  double updateWeights(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const;
};

extern BatchLearnerPtr adaBoostLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
extern BatchLearnerPtr adaBoostMHLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
