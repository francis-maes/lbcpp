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
# include <lbcpp/Luape/LuapeCache.h> // tmp, for IndexSet

namespace lbcpp
{

class LuapeLearner : public Object
{
public:
  LuapeLearner() : verbose(false) {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);
  virtual bool doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore) = 0;

  const LuapeInferencePtr& getFunction() const
    {return function;}
  
  const LuapeNodeUniversePtr& getUniverse() const
    {return function->getUniverse();}

  const LuapeNodePtr& getRootNode() const
    {return function->getRootNode();}

  const LuapeSamplesCachePtr& getTrainingCache() const
    {return trainingCache;}

  const LuapeSamplesCachePtr& getValidationCache() const
    {return validationCache;}

  VectorPtr getTrainingPredictions() const;
  VectorPtr getValidationPredictions() const;

  void setVerbose(bool v)
    {verbose = v;}

  bool getVerbose() const
    {return verbose;}

protected:
  friend class LuapeLearnerClass;

  bool verbose;

  LuapeInferencePtr function;
  std::vector<ObjectPtr> trainingData;
  LuapeSamplesCachePtr trainingCache;
  std::vector<ObjectPtr> validationData;
  LuapeSamplesCachePtr validationCache;
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

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& indices, double& weakObjective) const = 0;

  virtual void observeObjectiveValue(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
    {}

  double computeWeakObjectiveWithEventualStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, LuapeNodePtr& weakNode, const IndexSetPtr& indices) const;
  double computeWeakObjective(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& indices) const;
  double computeWeakObjectiveWithStump(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, const IndexSetPtr& indices, double& bestThreshold) const;

  LuapeNodePtr makeStump(const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& numberNode, double threshold) const;
  LuapeNodePtr makeContribution(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& examples) const;
};

typedef ReferenceCountedObjectPtr<BoostingWeakLearner> BoostingWeakLearnerPtr;

class FiniteBoostingWeakLearner : public BoostingWeakLearner
{
public:
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
    {jassert(false); return false;} // this must be implemented

  virtual LuapeNodePtr learn(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, const IndexSetPtr& examples, double& weakObjective) const;
};

typedef ReferenceCountedObjectPtr<FiniteBoostingWeakLearner> FiniteBoostingWeakLearnerPtr;

class StochasticFiniteBoostingWeakLearner : public FiniteBoostingWeakLearner
{
public:
  StochasticFiniteBoostingWeakLearner(size_t numWeakNodes = 0)
    : numWeakNodes(numWeakNodes) {}
  
  virtual LuapeNodePtr sampleWeakNode(ExecutionContext& context, const BoostingLearnerPtr& structureLearner) const = 0;

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const BoostingLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const;

protected:
  friend class StochasticFiniteBoostingWeakLearnerClass;

  size_t numWeakNodes;
};

typedef ReferenceCountedObjectPtr<StochasticFiniteBoostingWeakLearner> StochasticFiniteBoostingWeakLearnerPtr;

extern FiniteBoostingWeakLearnerPtr constantWeakLearner();
extern FiniteBoostingWeakLearnerPtr singleStumpWeakLearner();
extern StochasticFiniteBoostingWeakLearnerPtr policyBasedWeakLearner(const PolicyPtr& policy, size_t budget, size_t maxDepth);
extern StochasticFiniteBoostingWeakLearnerPtr adaptativeSamplingWeakLearner(size_t numWeakNodes, size_t maxSteps);
extern FiniteBoostingWeakLearnerPtr exhaustiveWeakLearner(size_t maxDepth);

extern BoostingWeakLearnerPtr binaryTreeWeakLearner(BoostingWeakLearnerPtr conditionLearner, BoostingWeakLearnerPtr subLearner);
extern BoostingWeakLearnerPtr compositeWeakLearner(const std::vector<BoostingWeakLearnerPtr>& weakLearners);
extern BoostingWeakLearnerPtr compositeWeakLearner(BoostingWeakLearnerPtr weakLearner1, BoostingWeakLearnerPtr weakLearner2);
extern BoostingWeakLearnerPtr laminatingWeakLearner(BoostingWeakLearnerPtr weakLearner, size_t numInitialExamples = 10);

extern PolicyPtr treeBasedRandomPolicy();

class BoostingWeakObjective : public Object
{
public:
  virtual void setPredictions(const LuapeSampleVectorPtr& predictions) = 0;
  virtual void flipPrediction(size_t index) = 0; // flip from negative prediction to positive prediction
  virtual double computeObjective() const = 0;

  // these two functions have side effects on the currently stored predictions
  double compute(const LuapeSampleVectorPtr& predictions);
  double findBestThreshold(ExecutionContext& context, const IndexSetPtr& indices, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore, bool verbose = false);

private:
  SparseDoubleVectorPtr computeSortedDoubleValues(ExecutionContext& context, const LuapeSampleVectorPtr& samples) const;
};

typedef ReferenceCountedObjectPtr<BoostingWeakObjective> BoostingWeakObjectivePtr;

class BoostingLearner : public LuapeLearner
{
public:
  BoostingLearner(BoostingWeakLearnerPtr weakLearner);
  BoostingLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective() const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);

  const BoostingWeakLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  virtual bool doLearningIteration(ExecutionContext& context, double& trainingScore, double& validationScore);
  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const = 0;

  LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& examples) const;

protected:
  friend class BoostingLearnerClass;
  
  BoostingWeakLearnerPtr weakLearner;
};

extern BoostingLearnerPtr adaBoostLearner(BoostingWeakLearnerPtr weakLearner);
extern BoostingLearnerPtr adaBoostMHLearner(BoostingWeakLearnerPtr weakLearner, bool useSymmetricVotes);
extern BoostingLearnerPtr l2BoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate);
extern BoostingLearnerPtr rankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate, RankingLossFunctionPtr rankingLoss);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
