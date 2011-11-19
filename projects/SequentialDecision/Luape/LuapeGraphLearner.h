/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphLearner.h            | Luape Graph Learners            |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_LEARNER_H_
# define LBCPP_LUAPE_GRAPH_LEARNER_H_

# include "LuapeInference.h"
# include "LuapeGraphBuilder.h"
# include "LuapeProblem.h"

namespace lbcpp
{

class LuapeGraphLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual bool doLearningIteration(ExecutionContext& context) = 0;

  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const = 0;

protected:
  LuapeProblemPtr problem;
  LuapeInferencePtr function;
  LuapeGraphPtr graph;
};

typedef ReferenceCountedObjectPtr<LuapeGraphLearner> LuapeGraphLearnerPtr;

class LuapeGradientBoostingLearner : public LuapeGraphLearner
{
public:
  LuapeGradientBoostingLearner(double learningRate, size_t maxDepth);
  LuapeGradientBoostingLearner() : learningRate(0.0), maxDepth(maxDepth) {}

  // gradient boosting
  virtual LuapeNodePtr doWeakLearning(ExecutionContext& context, const DenseDoubleVectorPtr& predictions) const = 0;
  virtual bool addWeakLearnerToGraph(ExecutionContext& context, const DenseDoubleVectorPtr& predictions, LuapeNodePtr completion);

  // LuapeGraphLearner
  virtual bool doLearningIteration(ExecutionContext& context);
  virtual double computeCompletionReward(ExecutionContext& context, const LuapeNodePtr& completion) const;

protected:
  double learningRate;
  size_t maxDepth;

  DenseDoubleVectorPtr pseudoResiduals;
};

typedef ReferenceCountedObjectPtr<LuapeGradientBoostingLearner> LuapeGradientBoostingLearnerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_H_
