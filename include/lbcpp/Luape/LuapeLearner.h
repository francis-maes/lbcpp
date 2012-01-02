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
# include "BoostingWeakLearner.h"
# include <lbcpp/Learning/LossFunction.h>

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
  
  const LuapeUniversePtr& getUniverse() const
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

extern BoostingLearnerPtr discreteAdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner);
extern BoostingLearnerPtr realAdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner);

extern BoostingLearnerPtr l2BoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate);
extern BoostingLearnerPtr rankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, double learningRate, RankingLossFunctionPtr rankingLoss);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
