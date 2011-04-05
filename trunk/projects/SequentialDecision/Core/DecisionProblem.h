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

class DecisionProblemState : public NameableObject
{
public:
  DecisionProblemState(const String& name)
    : NameableObject(name) {}
  DecisionProblemState() {}

  virtual TypePtr getActionType() const = 0;
  virtual ContainerPtr getAvailableActions() const = 0;
  virtual void performTransition(const Variable& action, double& reward) = 0;

  virtual bool isFinalState() const
    {return !getAvailableActions();}

  void performTrajectory(const ContainerPtr& actions, double& sumOfRewards, size_t maxSteps = 0);
  bool checkTrajectoryValidity(ExecutionContext& context, const ContainerPtr& trajectory) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<DecisionProblemState> DecisionProblemStatePtr;

extern ClassPtr decisionProblemStateClass;

class DecisionProblem : public Object
{
public:
  DecisionProblem(const FunctionPtr& initialStateSampler, double discount)
    : initialStateSampler(initialStateSampler), discount(discount)
    {initialStateSampler->initialize(defaultExecutionContext(), randomGeneratorClass);}
  DecisionProblem() {}

  /*
  ** Initial states
  */
  TypePtr getStateType() const
    {return initialStateSampler->getOutputType();}

  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  DecisionProblemStatePtr sampleInitialState(RandomGeneratorPtr random) const
    {return initialStateSampler->compute(defaultExecutionContext(), random).getObjectAndCast<DecisionProblemState>();}

  /*
  ** Objective
  */
  double getDiscount() const
    {return discount;} // in [0,1]

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