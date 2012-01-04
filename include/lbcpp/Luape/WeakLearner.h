/*-----------------------------------------.---------------------------------.
| Filename: WeakLearner.h          | Boosting Weak Learner           |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BOOSTING_WEAK_LEARNER_H_
# define LBCPP_LUAPE_BOOSTING_WEAK_LEARNER_H_

# include "LuapeInference.h"
# include "LuapeNodeBuilder.h"
# include "LuapeLearner.h"
# include <lbcpp/Data/IndexSet.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/DecisionProblem/Policy.h>
# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{

class WeakLearnerObjective : public Object
{
public:
  virtual void setPredictions(const LuapeSampleVectorPtr& predictions) = 0;
  virtual void flipPrediction(size_t index) = 0; // flip from negative prediction to positive prediction
  virtual double computeObjective() const = 0;

  // these two functions have side effects on the currently stored predictions
  double compute(const LuapeSampleVectorPtr& predictions);
  double findBestThreshold(ExecutionContext& context, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose = false);
};

typedef ReferenceCountedObjectPtr<WeakLearnerObjective> WeakLearnerObjectivePtr;

class WeakLearner : public LuapeLearner
{
public:
  WeakLearner() : bestWeakObjectiveValue(-DBL_MAX) {}

  void setWeakObjective(const WeakLearnerObjectivePtr& weakObjective)
    {this->weakObjective = weakObjective;}

  double computeWeakObjectiveWithEventualStump(ExecutionContext& context, const LuapeInferencePtr& problem, LuapeNodePtr& weakNode, const IndexSetPtr& indices) const;
  double computeWeakObjective(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& weakNode, const IndexSetPtr& indices) const;
  double computeWeakObjectiveWithStump(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, double& bestThreshold) const;

  double getBestWeakObjectiveValue() const
    {return bestWeakObjectiveValue;}

protected:
  WeakLearnerObjectivePtr weakObjective;
  double bestWeakObjectiveValue;
};

typedef ReferenceCountedObjectPtr<WeakLearner> WeakLearnerPtr;

extern WeakLearnerPtr exactWeakLearner(LuapeNodeBuilderPtr nodeBuilder);
extern WeakLearnerPtr laminatingWeakLearner(LuapeNodeBuilderPtr nodeBuilder, double relativeBudget, size_t minExamplesForLaminating = 5);
extern WeakLearnerPtr banditBasedWeakLearner(LuapeNodeBuilderPtr nodeBuilder, double relativeBudget, double miniBatchRelativeSize = 0.01);
extern WeakLearnerPtr optimizerBasedSequentialWeakLearner(OptimizerPtr optimizer, size_t complexity);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
