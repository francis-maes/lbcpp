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

  virtual void setFunction(const LuapeInferencePtr& function)
    {this->function = function;}
  virtual bool setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data);

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context)
    {jassert(false); return LuapeNodePtr();}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node) = 0;

  const LuapeInferencePtr& getFunction() const
    {return function;}

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

  void evaluatePredictions(ExecutionContext& context, double& trainingScore, double& validationScore);
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

extern LuapeLearnerPtr ensembleLearner(const LuapeLearnerPtr& baseLearner, size_t ensembleSize);
extern LuapeLearnerPtr compositeLearner(const std::vector<LuapeLearnerPtr>& learners);
extern LuapeLearnerPtr compositeLearner(const LuapeLearnerPtr& learner1, const LuapeLearnerPtr& learner2);

extern LuapeLearnerPtr generateTestNodesLearner(LuapeNodeBuilderPtr nodeBuilder);

class IterativeLearner : public LuapeLearner
{
public:
  IterativeLearner(size_t maxIterations = 0);
  virtual ~IterativeLearner();

  void setPlotFile(ExecutionContext& context, const File& plotFile);

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node);
  
  OutputStream* getPlotOutputStream() const
    {return plotOutputStream;}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node)
    {return true;}
  virtual bool doLearningIteration(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& rootNode, double& trainingScore, double& validationScore) = 0;
  virtual bool finalize(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node)
    {return true;}

protected:
  friend class IterativeLearnerClass;
  
  size_t maxIterations;

  OutputStream* plotOutputStream;
  
  void getImportances(const LuapeNodePtr& node, std::map<LuapeNodePtr, double>& res) const;
  void displayMostImportantNodes(ExecutionContext& context, const LuapeInferencePtr& function, bool verbose) const;
};

extern IterativeLearnerPtr classifierSGDLearner(MultiClassLossFunctionPtr lossFunction, IterationFunctionPtr learningRate, size_t maxIterations);

class BoostingLearner : public IterativeLearner
{
public:
  BoostingLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations);
  BoostingLearner() {}

  virtual BoostingWeakObjectivePtr createWeakObjective() const = 0;

  const BoostingWeakLearnerPtr& getWeakLearner() const
    {return weakLearner;}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& node);
  virtual bool doLearningIteration(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& rootNode, double& trainingScore, double& validationScore);
  virtual bool computeVotes(ExecutionContext& context, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, Variable& successVote, Variable& failureVote, Variable& missingVote) const = 0;

  LuapeNodePtr turnWeakNodeIntoContribution(ExecutionContext& context, const LuapeNodePtr& weakNode, double weakObjective, const IndexSetPtr& examples) const;

protected:
  friend class BoostingLearnerClass;
  
  BoostingWeakLearnerPtr weakLearner;
};

extern BoostingLearnerPtr adaBoostLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations);

extern BoostingLearnerPtr discreteAdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations);
extern BoostingLearnerPtr realAdaBoostMHLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations);

extern BoostingLearnerPtr l2BoostingLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations, double learningRate);
extern BoostingLearnerPtr rankingGradientBoostingLearner(BoostingWeakLearnerPtr weakLearner, size_t maxIterations, double learningRate, RankingLossFunctionPtr rankingLoss);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
