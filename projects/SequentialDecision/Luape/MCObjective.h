/*-----------------------------------------.---------------------------------.
| Filename: MCObjective.h                  | Monte Carlo Objective Functions |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2012 15:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_OBJECTIVE_H_
# define LBCPP_LUAPE_MC_OBJECTIVE_H_

# include <lbcpp/Luape/LuapeInference.h>
# include "../../../src/Luape/NodeBuilder/NodeBuilderDecisionProblem.h"

namespace lbcpp
{

class MCObjective : public Object
{
public:
  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState) = 0;
  virtual bool shouldStop() const
    {return false;}
};

typedef ReferenceCountedObjectPtr<MCObjective> MCObjectivePtr;

class LuapeMCObjective : public MCObjective
{
public:
  LuapeMCObjective(LuapeInferencePtr problem = LuapeInferencePtr())
    : problem(problem) {}

  virtual double evaluate(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& formula) = 0;

  virtual double evaluate(ExecutionContext& context, DecisionProblemStatePtr finalState)
  {
    LuapeNodeBuilderStatePtr builder = finalState.staticCast<LuapeNodeBuilderState>();
    if (builder->getStackSize() != 1)
      return -DBL_MAX;
    return evaluate(context, problem, builder->getStackElement(0));
  }

protected:
  friend class LuapeMCObjectiveClass;

  LuapeInferencePtr problem;
};

class SymbolicRegressionMCObjective : public LuapeMCObjective
{
public:
  SymbolicRegressionMCObjective(LuapeInferencePtr problem = LuapeInferencePtr())
    : LuapeMCObjective(problem) {}

  virtual double evaluate(ExecutionContext& context, const LuapeInferencePtr& problem, const LuapeNodePtr& formula)
  {
    DenseDoubleVectorPtr supervisions = problem->getTrainingSupervisions();
    LuapeSampleVectorPtr predictions = problem->getTrainingCache()->getSamples(context, formula);
    ScalarVariableMean res;
    for (LuapeSampleVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      double prediction = it.getRawDouble();
      if (prediction == doubleMissingValue)
        return -DBL_MAX;
      res.push(fabs(supervisions->getValue(it.getIndex()) - prediction));
    }
    return -res.getMean();
  }
};

typedef ReferenceCountedObjectPtr<SymbolicRegressionMCObjective> SymbolicRegressionMCObjectivePtr;


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_OBJECTIVE_H_
