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
  virtual std::vector<LuapeNodePtr> learn(ExecutionContext& context, const BoostingLuapeLearnerPtr& batchLearner, const LuapeFunctionPtr& function,
                                          const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
};

typedef ReferenceCountedObjectPtr<LuapeWeakLearner> LuapeWeakLearnerPtr;

extern LuapeWeakLearnerPtr singleStumpWeakLearner();
extern LuapeWeakLearnerPtr productWeakLearner(LuapeWeakLearnerPtr baseLearner, size_t numBaseClassifiers);
extern LuapeWeakLearnerPtr luapeGraphBuilderWeakLearner(OptimizerPtr optimizer, size_t maxSteps);

extern LuapeWeakLearnerPtr combinedStumpWeakLearner();

/*
** BoostingEdgeCalculator
*/
class BoostingEdgeCalculator : public Object
{
public:
  virtual void initialize(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) = 0;
  virtual void flipPrediction(size_t index) = 0;
  virtual double computeEdge() const = 0;
  virtual Variable computeVote() const = 0;
  
  double findBestThreshold(ExecutionContext& context, LuapeNodePtr node, double& edge, bool verbose = false); // this modifies the prediction vector
};

typedef ReferenceCountedObjectPtr<BoostingEdgeCalculator> BoostingEdgeCalculatorPtr;


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

  virtual BoostingEdgeCalculatorPtr createEdgeCalculator() const = 0;

  virtual DenseDoubleVectorPtr makeInitialWeights(const LuapeFunctionPtr& function, const std::vector<PairPtr>& examples) const = 0;
  virtual VectorPtr makeSupervisions(const std::vector<ObjectPtr>& examples) const;

  // the absolute value of this quantity should be maximized
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const LuapeFunctionPtr& function, size_t index, double currentWeight, const BooleanVectorPtr& prediction, const ContainerPtr& supervision, const Variable& vote) const = 0;
  virtual double computeError(const ContainerPtr& predictions, const ContainerPtr& supervisions) const = 0;

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;

protected:
  friend class BoostingLuapeLearnerClass;

  LuapeWeakLearnerPtr weakLearner;
  size_t maxIterations;

  void addExamplesToGraph(bool areTrainingSamples, const std::vector<ObjectPtr>& examples, LuapeGraphPtr graph) const;
  double updateWeights(const LuapeFunctionPtr& function, const BooleanVectorPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, const Variable& vote) const;
  void updatePredictions(const LuapeFunctionPtr& function, VectorPtr predictions, const BooleanVectorPtr& weakPredictions, const Variable& vote) const;
};

extern BatchLearnerPtr adaBoostLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);
extern BatchLearnerPtr adaBoostMHLuapeLearner(LuapeProblemPtr problem, LuapeWeakLearnerPtr weakLearner, size_t maxIterations);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_BATCH_LEARNER_H_
