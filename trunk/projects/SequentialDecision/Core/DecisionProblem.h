/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.h              | Sequential Decision System      |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_SYSTEM_H_
# define LBCPP_SEQUENTIAL_DECISION_SYSTEM_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class DecisionProblemState : public NameableObject
{
public:
  DecisionProblemState(const String& name)
    : NameableObject(name) {}
  DecisionProblemState() {}

  virtual TypePtr getActionType() const = 0;
  virtual ContainerPtr getAvailableActions() const = 0;
  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL) = 0;
  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
    {return false;}

  virtual bool isFinalState() const
    {return !getAvailableActions();}

  void performTrajectory(const ContainerPtr& actions, double& sumOfRewards, size_t maxSteps = 0);
  bool checkTrajectoryValidity(ExecutionContext& context, const ContainerPtr& trajectory) const;

  // lua
  static int getActions(LuaState& state);
  static int isFinal(LuaState& state);
  static int performTransition(LuaState& state);

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<DecisionProblemState> DecisionProblemStatePtr;

extern ClassPtr decisionProblemStateClass;

class DecisionProblem : public Object
{
public:
  DecisionProblem(const FunctionPtr& initialStateSampler, double discount)
    : initialStateSampler(initialStateSampler), discount(discount)
    {if (initialStateSampler) initialStateSampler->initialize(defaultExecutionContext(), randomGeneratorClass);}
  DecisionProblem() {}

  /*
  ** Initial states
  */
  ClassPtr getStateClass() const;

  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  DecisionProblemStatePtr sampleInitialState(ExecutionContext& context, RandomGeneratorPtr random) const;
  ContainerPtr sampleInitialStates(ExecutionContext& context, RandomGeneratorPtr random, size_t count) const;

  /*
  ** Actions
  */
  virtual TypePtr getActionType() const = 0;
  virtual size_t getFixedNumberOfActions() const
    {return 0;}

  /*
  ** Objective
  */
  double getDiscount() const
    {return discount;} // in [0,1]

  virtual double getMaxReward() const
    {return 1.0;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DecisionProblemClass;

  FunctionPtr initialStateSampler;
  double discount;
};

typedef ReferenceCountedObjectPtr<DecisionProblem> DecisionProblemPtr;

extern ClassPtr decisionProblemClass;


// State -> Container[Action]
class GetAvailableActionsFunction : public SimpleUnaryFunction
{
public:
  GetAvailableActionsFunction(TypePtr actionType = variableType)
    : SimpleUnaryFunction(decisionProblemStateClass, containerClass(actionType), T("Actions")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DecisionProblemStatePtr& state = input.getObjectAndCast<DecisionProblemState>();
    return state->getAvailableActions();
  }

  lbcpp_UseDebuggingNewOperator
};

extern FunctionPtr getAvailableActionsFunction(TypePtr actionType);

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_
