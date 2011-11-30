/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.h                 | Luape Graph Learners            |
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
# include "../Core/Policy.h"

namespace lbcpp
{

class LuapeLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual bool doLearningIteration(ExecutionContext& context) = 0;

  const LuapeProblemPtr& getProblem() const
    {return problem;}
    
  const LuapeInferencePtr& getFunction() const
    {return function;}
    
  const LuapeGraphPtr& getGraph() const
    {return graph;}

protected:
  LuapeProblemPtr problem;
  LuapeInferencePtr function;
  LuapeGraphPtr graph;

  std::vector<ObjectPtr> trainData;
  std::vector<ObjectPtr> validationData;
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class BoostingLearner;
typedef ReferenceCountedObjectPtr<BoostingLearner> BoostingLearnerPtr;

class BoostingWeakLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function) {return true;}
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples) const = 0;
  virtual void update(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr weakLearner) {}
};

typedef ReferenceCountedObjectPtr<BoostingWeakLearner> BoostingWeakLearnerPtr;

extern BoostingWeakLearnerPtr singleStumpWeakLearner();
extern BoostingWeakLearnerPtr policyBasedWeakLearner(const PolicyPtr& policy, size_t budget, size_t maxDepth);

class BoostingWeakObjective : public Object
{
public:
  virtual void setPredictions(const VectorPtr& predictions) = 0;
  virtual void flipPrediction(size_t index) = 0; // only valid if predictions are booleans
  virtual double computeObjective() const = 0;

  // these two functions have side effects on the currently stored predictions
  double compute(const VectorPtr& predictions);
  double findBestThreshold(ExecutionContext& context, const SparseDoubleVectorPtr& sortedDoubleValues, double& edge, bool verbose = false);
};

typedef ReferenceCountedObjectPtr<BoostingWeakObjective> BoostingWeakObjectivePtr;

class BoostingLearner : public LuapeLearner
{
public:
  BoostingLearner(BoostingWeakLearnerPtr weakLearner);
  BoostingLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);

  const BoostingWeakLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const;
  double computeBestStumpThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode, const std::vector<size_t>& examples) const;

  LuapeNodePtr doWeakLearning(ExecutionContext& context, const BoostingWeakLearnerPtr& weakLearner, const std::vector<size_t>& examples) const;

protected:
  friend class BoostingLearnerClass;
  
  BoostingWeakLearnerPtr weakLearner;
  VectorPtr predictions;
  VectorPtr validationPredictions;
  std::vector<size_t> allExamples;

  LuapeNodePtr createDecisionStump(ExecutionContext& context, const LuapeNodePtr& numberNode, const std::vector<size_t>& examples) const;
  LuapeNodePtr doWeakLearningAndAddToGraph(ExecutionContext& context, VectorPtr& weakPredictions);
  void updatePredictionsAndEvaluate(ExecutionContext& context, size_t yieldIndex, const LuapeNodePtr& weakNode) const;
  void recomputePredictions(ExecutionContext& context);
  double getSignedScalarPrediction(const VectorPtr& predictions, size_t index) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_H_
