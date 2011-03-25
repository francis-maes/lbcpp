/*-----------------------------------------.---------------------------------.
| Filename: Policy.h                       | Policy Base classes             |
| Author  : Francis Maes                   |                                 |
| Started : 05/03/2011 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_CORE_POLICY_H_
# define LBCPP_SEQUENTIAL_DECISION_CORE_POLICY_H_

# include "SearchTree.h"

namespace lbcpp
{

class Policy : public Object
{
public:
  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions) = 0;
  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions) = 0;
  virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState) {}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Policy> PolicyPtr;

extern PolicyPtr mixturePolicy(const PolicyPtr& policy1, const PolicyPtr& policy2, double k);

class MixturePolicy : public Policy
{
public:
  MixturePolicy(const PolicyPtr& policy1, const PolicyPtr& policy2, double k)
    : policy1(policy1), policy2(policy2), k(k), random(new RandomGenerator()) {}
  MixturePolicy() : k(0.0) {}

  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions)
  {
    Variable a1 = policy1->policyStart(context, state, actions);
    Variable a2 = policy2->policyStart(context, state, actions);
    return sampleAction(a1, a2);
  }

  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions)
  {
    Variable a1 = policy1->policyStep(context, reward, state, actions);
    Variable a2 = policy2->policyStep(context, reward, state, actions);
    return sampleAction(a1, a2);
  }

  virtual void policyEnd(ExecutionContext& context, double reward, const Variable& finalState)
  {
    policy1->policyEnd(context, reward, finalState);
    policy2->policyEnd(context, reward, finalState);
  }

protected:
  friend class MixturePolicyClass;

  PolicyPtr policy1;
  PolicyPtr policy2;
  double k;
  RandomGeneratorPtr random;

  Variable sampleAction(const Variable& a1, const Variable& a2)
    {return random->sampleBool(k) ? a2 : a1;}
};

#if 0
class ReproduceTrajectoryPolicy : public Policy
{
public:
  ReproduceTrajectoryPolicy(const ContainerPtr& trajectory = ContainerPtr())
    : trajectory(trajectory) {}

  virtual Variable policyStart(ExecutionContext& context, const Variable& state, const ContainerPtr& actions)
  {
    timeStep = 0;
    return getTrajectoryAction(timeStep);
  }

  virtual Variable policyStep(ExecutionContext& context, double reward, const Variable& state, const ContainerPtr& actions)
  {
    ++timeStep;
    return getTrajectoryAction(timeStep);
  }

protected:
  friend class ReproduceTrajectoryPolicyClass;

  ContainerPtr trajectory;
  size_t timeStep;

  Variable getTrajectoryAction(size_t timeStep) const
  {
    if (timeStep < trajectory->getNumElements())
      return trajectory->getElement(timeStep);
    else
      return Variable();
  }
};
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_CORE_POLICY_H_
