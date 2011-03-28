/*-----------------------------------------.---------------------------------.
| Filename: HIVSandBox.h                   | HIV Sand Box                    |
| Author  : Francis Maes                   |                                 |
| Started : 28/03/2011 13:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_HIV_SAND_BOX_H_

# include "../Problem/DamienDecisionProblem.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class HIVSandBox : public WorkUnit
{
public:
  // default values
  HIVSandBox()
  {
  }

  virtual Variable run(ExecutionContext& context)
  {
    DecisionProblemPtr problem = hivDecisionProblem(1.0);
    if (!problem)
    {
      context.errorCallback(T("No decision problem"));
      return false;
    }

    DecisionProblemStatePtr state = problem->sampleInitialState(RandomGenerator::getInstance());
    Variable action = state->getAvailableActions()->getElement(0);
    for (size_t i = 0; i < 50; ++i)
    {
      double reward;
      state->performTransition(action, reward);

      context.enterScope(T("Step ") + String((int)i + 1));
      context.resultCallback(T("step"), i + 1);
      context.resultCallback(T("log10(reward)"), log10(reward));
      for (size_t j = 0; j < 6; ++j)
        context.resultCallback(T("log10(state") + String((int)j + 1) + T(")"), log10(state.staticCast<DamienState>()->getStateDimension(j)));
      context.leaveScope(reward);
    }
    return true;
  }

private:
  friend class HIVSandBoxClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_SAND_BOX_H_
