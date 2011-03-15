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

  void performTrajectory(const ContainerPtr& actions, double& sumOfRewards);
  bool checkTrajectoryValidity(ExecutionContext& context, const ContainerPtr& trajectory) const;
};

typedef ReferenceCountedObjectPtr<DecisionProblemState> DecisionProblemStatePtr;

class DecisionProblem : public Object
{
public:
  DecisionProblem(const FunctionPtr& initialStateSampler, double discount)
    : initialStateSampler(initialStateSampler), discount(discount)
  {
  }
  DecisionProblem() {}

  /*
  ** Initial states
  */
  const TypePtr& getStateType() const
    {return initialStateSampler->getOutputType();}

  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  Variable sampleInitialState(RandomGeneratorPtr random) const
    {return initialStateSampler->compute(defaultExecutionContext(), random);}

  /*
  ** Objective
  */
  double getDiscount() const
    {return discount;} // in [0,1]

protected:
  friend class DecisionProblemClass;

  FunctionPtr initialStateSampler;
  double discount;
};

typedef ReferenceCountedObjectPtr<DecisionProblem> DecisionProblemPtr;

extern ClassPtr decisionProblemClass;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_