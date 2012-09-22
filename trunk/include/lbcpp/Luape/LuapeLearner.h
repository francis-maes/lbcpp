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
# include "LuapeNodeBuilder.h"
# include "LearningObjective.h"
# include "../Learning/LossFunction.h"
# include "../Data/IterationFunction.h"

namespace lbcpp
{

class LuapeLearner : public Object
{
public:
  LuapeLearner(const LearningObjectivePtr& objective = LearningObjectivePtr())
    : objective(objective), verbose(false), bestObjectiveValue(-DBL_MAX) {}

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
    {return LuapeNodePtr();}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples) = 0;

  LuapeNodePtr learn(ExecutionContext& context, const LuapeInferencePtr& problem, const IndexSetPtr& examples = IndexSetPtr());

  void setObjective(const LearningObjectivePtr& objective)
    {this->objective = objective;}

  const LearningObjectivePtr& getObjective() const
    {return objective;}

  void setVerbose(bool v)
    {verbose = v;}

  bool getVerbose() const
    {return verbose;}

  double getBestObjectiveValue() const
    {return bestObjectiveValue;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class LuapeLearnerClass;

  LearningObjectivePtr objective;
  bool verbose;
  double bestObjectiveValue;

  void evaluatePredictions(ExecutionContext& context, const LuapeInferencePtr& problem, double& trainingScore, double& validationScore);

  LuapeNodePtr subLearn(ExecutionContext& context, const LuapeLearnerPtr& subLearner, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double* objectiveValue = NULL) const;
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class IterativeLearner : public LuapeLearner
{
public:
  IterativeLearner(const LearningObjectivePtr& objective = LearningObjectivePtr(), size_t maxIterations = 0);
  virtual ~IterativeLearner();

  void setPlotFile(ExecutionContext& context, const File& plotFile);

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples);
  
  OutputStream* getPlotOutputStream() const
    {return plotOutputStream;}

  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
    {if (objective) objective->initialize(problem); return true;}
  virtual bool doLearningIteration(ExecutionContext& context, LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore) = 0;
  virtual bool finalize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
    {return true;}

protected:
  friend class IterativeLearnerClass;
  
  size_t maxIterations;

  OutputStream* plotOutputStream;
};

class NodeBuilderBasedLearner : public LuapeLearner
{
public:
  NodeBuilderBasedLearner(LuapeNodeBuilderPtr nodeBuilder);
  NodeBuilderBasedLearner() {}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  const LuapeNodeBuilderPtr& getNodeBuilder() const
    {return nodeBuilder;}

protected:
  friend class NodeBuilderBasedLearnerClass;

  LuapeNodeBuilderPtr nodeBuilder;
};


class DecoratorLearner : public LuapeLearner
{
public:
  DecoratorLearner(LuapeLearnerPtr decorated = LuapeLearnerPtr());

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem);
  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples);
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class DecoratorLearnerClass;

  LuapeLearnerPtr decorated;
};

// gradient descent
extern IterativeLearnerPtr classifierSGDLearner(MultiClassLossFunctionPtr lossFunction, IterationFunctionPtr learningRate, size_t maxIterations);

// boosting
extern IterativeLearnerPtr adaBoostLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth = 1);
extern IterativeLearnerPtr discreteAdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth = 1);
extern IterativeLearnerPtr realAdaBoostMHLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, size_t treeDepth = 1);
extern IterativeLearnerPtr l2BoostingLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, double learningRate, size_t treeDepth = 1);
extern IterativeLearnerPtr rankingGradientBoostingLearner(LuapeLearnerPtr weakLearner, size_t maxIterations, double learningRate, RankingLossFunctionPtr rankingLoss, size_t treeDepth = 1);

// meta
extern LuapeLearnerPtr ensembleLearner(const LuapeLearnerPtr& baseLearner, size_t ensembleSize);
extern LuapeLearnerPtr baggingLearner(const LuapeLearnerPtr& baseLearner, size_t ensembleSize);
extern LuapeLearnerPtr compositeLearner(const std::vector<LuapeLearnerPtr>& learners);
extern LuapeLearnerPtr compositeLearner(const LuapeLearnerPtr& learner1, const LuapeLearnerPtr& learner2);
extern LuapeLearnerPtr treeLearner(LearningObjectivePtr weakObjective, LuapeLearnerPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth);

extern DecoratorLearnerPtr addActiveVariablesLearner(LuapeLearnerPtr decorated, size_t numActiveVariables, bool deterministic);

// misc
extern LuapeLearnerPtr generateTestNodesLearner(LuapeNodeBuilderPtr nodeBuilder);

// weak learners
extern NodeBuilderBasedLearnerPtr exactWeakLearner(LuapeNodeBuilderPtr nodeBuilder);
extern NodeBuilderBasedLearnerPtr randomSplitWeakLearner(LuapeNodeBuilderPtr nodeBuilder);
extern NodeBuilderBasedLearnerPtr laminatingWeakLearner(LuapeNodeBuilderPtr nodeBuilder, double relativeBudget, size_t minExamplesForLaminating = 5);
extern NodeBuilderBasedLearnerPtr banditBasedWeakLearner(LuapeNodeBuilderPtr nodeBuilder, double relativeBudget, double miniBatchRelativeSize = 0.01);
//extern LuapeLearnerPtr optimizerBasedSequentialWeakLearner(OptimizerPtr optimizer, size_t complexity, bool useRandomSplit = false);

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
