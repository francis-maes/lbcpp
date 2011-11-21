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

namespace lbcpp
{

class TimedScope
{
public:
  TimedScope(ExecutionContext& context, const String& name)
    : context(context), name(name), startTime(juce::Time::getMillisecondCounterHiRes()) {}

  ~TimedScope()
  {
    double endTime = juce::Time::getMillisecondCounterHiRes();
    context.resultCallback(name + T(" time"), endTime);
  }

private:
  ExecutionContext& context;
  String name;
  double startTime;
};

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
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class LuapeBoostingLearner;
typedef ReferenceCountedObjectPtr<LuapeBoostingLearner> LuapeBoostingLearnerPtr;

class LuapeWeakLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function) {return true;}
  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeBoostingLearnerPtr& structureLearner) const = 0;
  virtual void update(ExecutionContext& context, const LuapeBoostingLearnerPtr& structureLearner, LuapeNodePtr weakLearner) {}
};

typedef ReferenceCountedObjectPtr<LuapeWeakLearner> LuapeWeakLearnerPtr;

extern LuapeWeakLearnerPtr singleStumpWeakLearner();

class LuapeBoostingLearner : public LuapeLearner
{
public:
  LuapeBoostingLearner(LuapeWeakLearnerPtr weakLearner)
    : weakLearner(weakLearner) {}
  LuapeBoostingLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    if (!LuapeLearner::initialize(context, problem, function))
      return false;
    jassert(weakLearner);
    return weakLearner->initialize(context, problem, function);
  }

  virtual double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& weakNode) const = 0;
  virtual double computeBestStumpThreshold(ExecutionContext& context, const LuapeNodePtr& numberNode) const = 0;
  
protected:
  friend class LuapeBoostingLearnerClass;
  
  LuapeWeakLearnerPtr weakLearner;

  LuapeNodePtr createDecisionStump(ExecutionContext& context, const LuapeNodePtr& numberNode) const
  {
    double threshold = computeBestStumpThreshold(context, numberNode);
    return graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);
  }

  LuapeNodePtr doWeakLearning(ExecutionContext& context) const
  {
    LuapeNodePtr weakNode = weakLearner->learn(context, refCountedPointerFromThis(this));
    if (!weakNode)
      context.errorCallback(T("Failed to find a weak learner"));
    if (weakNode->getType() != booleanType)
      weakNode = createDecisionStump(context, weakNode); // transforms doubles into decision stumps
    return weakNode;
  }

  LuapeNodePtr doWeakLearningAndAddToGraph(ExecutionContext& context, BooleanVectorPtr& weakPredictions)
  {
    LuapeNodePtr weakNode;

    // do weak learning
    {
      TimedScope _(context, "weak learning");
      weakNode = doWeakLearning(context);
      if (!weakNode)
        return LuapeNodePtr();
    }
   
    {
      TimedScope _(context, "add to graph");

      // add missing nodes to graph
      jassert(weakNode->getType() == booleanType);
      graph->pushMissingNodes(context, new LuapeYieldNode(weakNode));

      // update the weak learner
      weakLearner->update(context, refCountedPointerFromThis(this), weakNode);

      // retrieve weak predictions
      weakNode->updateCache(context, true);
      jassert(weakNode->getCache()->getNumTrainingSamples() > 0);
      weakPredictions = weakNode->getCache()->getSamples(true).staticCast<BooleanVector>();
    }
    return weakNode;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_H_
