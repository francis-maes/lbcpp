/*-----------------------------------------.---------------------------------.
| Filename: BoostingWeakLearner.h          | Boosting Weak Learner           |
| Author  : Francis Maes                   |   base classes                  |
| Started : 22/12/2011 14:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_BOOSTING_WEAK_LEARNER_H_
# define LBCPP_LUAPE_BOOSTING_WEAK_LEARNER_H_

# include "LuapeInference.h"
# include <lbcpp/Data/IndexSet.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/DecisionProblem/Policy.h>

namespace lbcpp
{

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

class BoostingWeakLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function) {return true;}

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
    {return false;}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& indices, bool verbose, double& weakObjective) = 0;

  virtual void observeObjectiveValue(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
    {}
  virtual void observeBestWeakNode(ExecutionContext& context,  const LuapeLearnerPtr& structureLearner, const LuapeNodePtr& bestWeakNode, const IndexSetPtr& examples, double bestWeakObjective)
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
  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& candidates) const
    {jassert(false); return false;} // this must be implemented

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective);
};

typedef ReferenceCountedObjectPtr<FiniteBoostingWeakLearner> FiniteBoostingWeakLearnerPtr;

class StochasticFiniteBoostingWeakLearner : public FiniteBoostingWeakLearner
{
public:
  StochasticFiniteBoostingWeakLearner(size_t numWeakNodes = 0)
    : numWeakNodes(numWeakNodes) {}
  
  virtual LuapeNodePtr sampleWeakNode(ExecutionContext& context, const LuapeLearnerPtr& structureLearner) const = 0;

  virtual bool getCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const;

protected:
  friend class StochasticFiniteBoostingWeakLearnerClass;

  size_t numWeakNodes;
};

typedef ReferenceCountedObjectPtr<StochasticFiniteBoostingWeakLearner> StochasticFiniteBoostingWeakLearnerPtr;

class SequentialBuilderWeakLearner : public StochasticFiniteBoostingWeakLearner
{
public:
  SequentialBuilderWeakLearner(size_t numWeakNodes, size_t maxSteps);
  SequentialBuilderWeakLearner() {}

  virtual bool sampleAction(ExecutionContext& context, LuapeGraphBuilderTypeStatePtr typeState, ObjectPtr& res) const = 0;

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function);
  virtual LuapeNodePtr sampleWeakNode(ExecutionContext& context, const LuapeLearnerPtr& structureLearner) const;

protected:
  friend class SequentialBuilderWeakLearnerClass;

  virtual void samplingDone(ExecutionContext& context, size_t numSamplingFailures, size_t numFailuresAllowed) {}

  size_t maxSteps;

  LuapeUniversePtr universe;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;

  static bool isActionAvailable(ObjectPtr action, const std::vector<LuapeNodePtr>& stack);
  LuapeGraphBuilderTypeStatePtr getTypeState(size_t stepNumber, const std::vector<LuapeNodePtr>& stack) const;
  void executeAction(std::vector<LuapeNodePtr>& stack, const ObjectPtr& action) const;
};

class DecoratorBoostingWeakLearner : public BoostingWeakLearner
{
public:
  DecoratorBoostingWeakLearner(BoostingWeakLearnerPtr decorated)
    : decorated(decorated) {}
  DecoratorBoostingWeakLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeInferencePtr& function)
    {return decorated->initialize(context, function);}
  
  virtual void observeObjectiveValue(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
    {decorated->observeObjectiveValue(context, structureLearner, weakNode, examples, weakObjective);}

  virtual void observeBestWeakNode(ExecutionContext& context,  const LuapeLearnerPtr& structureLearner, const LuapeNodePtr& weakNode, const IndexSetPtr& examples, double weakObjective)
    {decorated->observeBestWeakNode(context, structureLearner, weakNode, examples, weakObjective);}

  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, const IndexSetPtr& examples, bool verbose, double& weakObjective)
    {return decorated->learn(context, structureLearner, examples, verbose, weakObjective);}

protected:
  friend class DecoratorBoostingWeakLearnerClass;

  BoostingWeakLearnerPtr decorated;
  
  bool getDecoratedCandidateWeakNodes(ExecutionContext& context, const LuapeLearnerPtr& structureLearner, std::vector<LuapeNodePtr>& res) const
  {
    context.enterScope(T("Generating candidate weak learners"));
    // make initial weak learners
    bool ok = decorated->getCandidateWeakNodes(context, structureLearner, res);
    context.leaveScope(res.size());
    if (!ok)
    {
      context.errorCallback(T("Could not get finite set of candidate weak nodes"));
      return false;
    }
    else
      return true;
  }
};

typedef ReferenceCountedObjectPtr<DecoratorBoostingWeakLearner> DecoratorBoostingWeakLearnerPtr;

extern FiniteBoostingWeakLearnerPtr constantWeakLearner();
extern FiniteBoostingWeakLearnerPtr singleStumpWeakLearner();
extern StochasticFiniteBoostingWeakLearnerPtr policyBasedWeakLearner(const PolicyPtr& policy, size_t budget, size_t complexity);
extern StochasticFiniteBoostingWeakLearnerPtr adaptativeSamplingWeakLearner(size_t numWeakNodes, size_t complexity, bool useVariableRelevancies, bool useExtendedVariables);
extern FiniteBoostingWeakLearnerPtr exhaustiveWeakLearner(size_t maxDepth);

extern BoostingWeakLearnerPtr binaryTreeWeakLearner(BoostingWeakLearnerPtr conditionLearner, BoostingWeakLearnerPtr subLearner);
extern BoostingWeakLearnerPtr compositeWeakLearner(const std::vector<BoostingWeakLearnerPtr>& weakLearners);
extern BoostingWeakLearnerPtr compositeWeakLearner(BoostingWeakLearnerPtr weakLearner1, BoostingWeakLearnerPtr weakLearner2);
extern DecoratorBoostingWeakLearnerPtr laminatingWeakLearner(BoostingWeakLearnerPtr weakLearner, double relativeBudget, size_t minExamplesForLaminating = 5);
extern DecoratorBoostingWeakLearnerPtr banditBasedWeakLearner(BoostingWeakLearnerPtr weakLearner, double relativeBudget, double miniBatchRelativeSize = 0.01);

extern PolicyPtr treeBasedRandomPolicy();

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_LEARNER_H_
