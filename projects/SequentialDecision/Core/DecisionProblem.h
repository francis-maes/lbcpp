/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.h    | Sequential Decision System      |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_SYSTEM_H_
# define LBCPP_SEQUENTIAL_DECISION_SYSTEM_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

class DecisionProblem : public Object
{
public:
  DecisionProblem(const FunctionPtr& initialStateSampler, const FunctionPtr& transitionFunction, const FunctionPtr& rewardFunction, double discount);
  DecisionProblem() {}

  /*
  ** Types
  */
  const TypePtr& getStateType() const
    {return stateType;}

  const TypePtr& getActionType() const
    {return actionType;}

  /*
  ** Functions
  */
  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  const FunctionPtr& getTransitionFunction() const
    {return transitionFunction;}

  const FunctionPtr& getRewardFunction() const
    {return rewardFunction;}

  bool initialize(ExecutionContext& context);

  /*
  ** Shortcuts
  */
  Variable sampleInitialState(RandomGeneratorPtr random) const
    {return initialStateSampler->compute(defaultExecutionContext(), random);}

  virtual void getAvailableActions(const Variable& state, std::vector<Variable>& actions) const;

  Variable computeTransition(const Variable& state, const Variable& action) const
    {return transitionFunction->compute(defaultExecutionContext(), state, action);}

  double computeReward(const Variable& state, const Variable& action) const
    {return rewardFunction->compute(defaultExecutionContext(), state, action).getDouble();}

  /*
  ** High level operations
  */
  Variable computeFinalState(ExecutionContext& context, const Variable& initialState, const ContainerPtr& trajectory) const;
  bool checkTrajectoryValidity(ExecutionContext& context, const Variable& initialState, const ContainerPtr& trajectory) const;

  /*
  ** Objective
  */
  double getDiscount() const
    {return discount;} // in [0,1]

protected:
  friend class DecisionProblemClass;

  FunctionPtr initialStateSampler;
  FunctionPtr transitionFunction;
  FunctionPtr rewardFunction;

  TypePtr stateType;
  TypePtr actionType;

  double discount;
};

typedef ReferenceCountedObjectPtr<DecisionProblem> DecisionProblemPtr;

extern ClassPtr decisionProblemClass;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_