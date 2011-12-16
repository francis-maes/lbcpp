/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.h                 | Luape Graph Learners            |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_LEARNER_H_
# define LBCPP_LUAPE_LEARNER_H_

# include "LuapeInference.h"
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

class LuapeLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual bool doLearningIteration(ExecutionContext& context) = 0;

  const LuapeInferencePtr& getFunction() const
    {return function;}
  
  const LuapeNodeUniversePtr& getUniverse() const
    {return function->getUniverse();}

  const LuapeNodePtr& getRootNode() const
    {return function->getRootNode();}

  const LuapeSamplesCachePtr& getTrainingSamples() const
    {return trainingSamples;}

  const LuapeSamplesCachePtr& getValidationSamples() const
    {return validationSamples;}

  VectorPtr getTrainingPredictions() const;
  VectorPtr getValidationPredictions() const;

protected:
  LuapeInferencePtr function;

  std::vector<ObjectPtr> trainingData;
  LuapeSamplesCachePtr trainingSamples;

  std::vector<ObjectPtr> validationData;
  LuapeSamplesCachePtr validationSamples;
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class BoostingLearner;
typedef ReferenceCountedObjectPtr<BoostingLearner> BoostingLearnerPtr;

class BoostingWeakLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function) {return true;}

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
    {return false;}

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const = 0;
  //virtual void update(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr weakLearner) {}

  double computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const;
  double computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const;
  double computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const std::vector<size_t>& examples, double& bestThreshold) const;

  LuapeNodePtr makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const;
  LuapeNodePtr makeContribution(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const;
};

typedef ReferenceCountedObjectPtr<BoostingWeakLearner> BoostingWeakLearnerPtr;

class FiniteBoostingWeakLearner : public BoostingWeakLearner
{
public:
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
    {jassert(false); return false;} // this must be implemented
  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const std::vector<size_t>& examples, double& weakObjective) const;
};

typedef ReferenceCountedObjectPtr<FiniteBoostingWeakLearner> FiniteBoostingWeakLearnerPtr;

extern FiniteBoostingWeakLearnerPtr constantWeakLearner();
extern FiniteBoostingWeakLearnerPtr singleStumpWeakLearner();
extern FiniteBoostingWeakLearnerPtr policyBasedWeakLearner(const PolicyPtr& policy, size_t budget, size_t maxDepth);
extern BoostingWeakLearnerPtr binaryTreeWeakLearner(BoostingWeakLearnerPtr conditionLearner, BoostingWeakLearnerPtr subLearner);
extern BoostingWeakLearnerPtr compositeWeakLearner(const std::vector<BoostingWeakLearnerPtr>& weakLearners);
extern BoostingWeakLearnerPtr compositeWeakLearner(BoostingWeakLearnerPtr weakLearner1, BoostingWeakLearnerPtr weakLearner2);
extern BoostingWeakLearnerPtr laminatingWeakLearner(BoostingWeakLearnerPtr weakLearner, size_t numInitialExamples = 10);

extern PolicyPtr treeBasedRandomPolicy();

class BoostingWeakObjective : public Object
{
public:
  virtual void setPredictions(const VectorPtr& predictions) = 0;
  virtual void flipPrediction(size_t index) = 0; // only valid if predictions are booleans
  virtual double computeObjective() const = 0;

  // these two functions have side effects on the currently stored predictions
  double compute(const VectorPtr& predictions);
  double findBestThreshold(ExecutionContext& context, size_t numSamples, const SparseDoubleVectorPtr& sortedDoubleValues, double& edge, bool verbose = false);
};

typedef ReferenceCountedObjectPtr<BoostingWeakObjective> BoostingWeakObjectivePtr;

class BoostingLearner : public LuapeLearner
{
public:
  BoostingLearner(BoostingWeakLearnerPtr weakLearner);
  BoostingLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective(const std::vector<size_t>& examples) const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);

  const BoostingWeakLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  virtual bool doLearningIteration(ExecutionContext& context);
  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const = 0;

  LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, const std::vector<size_t>& examples) const;

protected:
  friend class BoostingLearnerClass;
  
  BoostingWeakLearnerPtr weakLearner;
  std::vector<size_t> allExamples;
};

extern BoostingLearnerPtr adaBoostLearner(BoostingWeakLearnerPtr weakLearner);
extern BoostingLearnerPtr adaBoostMHLearner(BoostingWeakLearnerPtr weakLearner, bool useSymmetricVotes);
extern BoostingLearnerPtr l2BoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate);
extern BoostingLearnerPtr rankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate, RankingLossFunctionPtr rankingLoss);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
