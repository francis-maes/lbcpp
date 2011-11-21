/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.h            | Luape Graph Learners            |
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

class LuapeGreedyStructureLearner;
typedef ReferenceCountedObjectPtr<LuapeGreedyStructureLearner> LuapeGreedyStructureLearnerPtr;

class LuapeWeakLearner : public Object
{
public:
  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function) {return true;}
  virtual LuapeNodePtr learn(ExecutionContext& context, const LuapeGreedyStructureLearnerPtr& structureLearner) const = 0;
  virtual void update(ExecutionContext& context, const LuapeGreedyStructureLearnerPtr& structureLearner, LuapeNodePtr weakLearner) {}
};

typedef ReferenceCountedObjectPtr<LuapeWeakLearner> LuapeWeakLearnerPtr;

extern LuapeWeakLearnerPtr singleStumpWeakLearner();

class LuapeGreedyStructureLearner : public LuapeLearner
{
public:
  LuapeGreedyStructureLearner(LuapeWeakLearnerPtr weakLearner)
    : weakLearner(weakLearner) {}
  LuapeGreedyStructureLearner() {}

  virtual bool initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
  {
    if (!LuapeLearner::initialize(context, problem, function))
      return false;
    jassert(weakLearner);
    return weakLearner->initialize(context, problem, function);
  }

  virtual double computeWeakObjective(ExecutionContext& context, const LuapeNodePtr& completion) const = 0;
  
protected:
  friend class LuapeGreedyStructureLearnerClass;
  
  LuapeWeakLearnerPtr weakLearner;
  
  LuapeNodePtr doWeakLearning(ExecutionContext& context) const
    {return weakLearner->learn(context, refCountedPointerFromThis(this));}
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_LEARNER_H_
