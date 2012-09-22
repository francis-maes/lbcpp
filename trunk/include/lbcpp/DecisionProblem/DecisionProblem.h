/*-----------------------------------------.---------------------------------.
| Filename: DecisionProblem.h              | Sequential Decision System      |
| Author  : Francis Maes                   |                                 |
| Started : 22/02/2011 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_PROBLEM_PROBLEM_H_
# define LBCPP_DECISION_PROBLEM_PROBLEM_H_

# include "../Core/Function.h"
# include "../Data/RandomGenerator.h"

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
  virtual ObjectVectorPtr computeActionFeatures(ExecutionContext& context, const ContainerPtr& actions) const
    {jassert(false); return ObjectVectorPtr();}

  virtual void performTransition(ExecutionContext& context, const Variable& action, double& reward, Variable* stateBackup = NULL) = 0;
  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
    {return false;}

  virtual bool isFinalState() const
    {return !getAvailableActions();}
    
  virtual double getFinalStateReward() const
    {return 0.0;}

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
  DecisionProblem(const FunctionPtr& initialStateSampler, double discount, size_t horizon = 0)
    : initialStateSampler(initialStateSampler), discount(discount), horizon(horizon)
    {if (initialStateSampler) initialStateSampler->initialize(defaultExecutionContext(), randomGeneratorClass);}
  DecisionProblem() {}

  /*
  ** Initial states
  */
  virtual ClassPtr getStateClass() const;

  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  virtual DecisionProblemStatePtr sampleInitialState(ExecutionContext& context) const;
  virtual DecisionProblemStatePtr sampleAnyState(ExecutionContext& context) const
    {jassert(false); return DecisionProblemStatePtr();}

  ContainerPtr sampleInitialStates(ExecutionContext& context, size_t count) const;

  virtual ObjectVectorPtr getValidationInitialStates(size_t& numTrajectoriesToValidate) const
    {numTrajectoriesToValidate = 0; return ObjectVectorPtr();}

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

  size_t getHorizon() const
    {return horizon;}

  virtual double getMaxReward() const
    {return 1.0;}

  virtual double getMaxCumulativeReward() const;

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class DecisionProblemClass;

  FunctionPtr initialStateSampler;
  double discount;
  size_t horizon;
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

#endif // !LBCPP_DECISION_PROBLEM_PROBLEM_H_
