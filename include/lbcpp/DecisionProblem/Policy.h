/*-----------------------------------------.---------------------------------.
| Filename: Policy.h                       | Policy Base classes             |
| Author  : Francis Maes                   |                                 |
| Started : 05/03/2011 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_PROBLEM_POLICY_H_
# define LBCPP_DECISION_PROBLEM_POLICY_H_

# include "DecisionProblem.h"

namespace lbcpp
{

class Policy : public Object
{
public:
  virtual void startEpisodes(ExecutionContext& context) {}
  virtual void startEpisode(ExecutionContext& context, const DecisionProblemStatePtr& initialState) = 0;

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state) = 0;

  virtual void observeTransition(ExecutionContext& context, const Variable& action, double reward, const DecisionProblemStatePtr& newState)
    {}

  virtual void finishEpisodes(ExecutionContext& context) {}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<Policy> PolicyPtr;

class RandomPolicy : public Policy
{
public:
  virtual void startEpisode(ExecutionContext& context, const DecisionProblemStatePtr& initialState)
    {}
  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    jassert(n != 0);
    return actions->getElement(context.getRandomGenerator()->sampleSize(n));
  }

  lbcpp_UseDebuggingNewOperator
};

extern PolicyPtr randomPolicy();
extern PolicyPtr mixturePolicy(const PolicyPtr& policy1, const PolicyPtr& policy2, double k);

class MixturePolicy : public Policy
{
public:
  MixturePolicy(const PolicyPtr& policy1, const PolicyPtr& policy2, double k)
    : policy1(policy1), policy2(policy2), k(k) {}
  MixturePolicy() : k(0.0) {}

  virtual void startEpisode(ExecutionContext& context, const DecisionProblemStatePtr& initialState)
  {
    policy1->startEpisode(context, initialState);
    policy2->startEpisode(context, initialState);
  }

  virtual Variable selectAction(ExecutionContext& context, const DecisionProblemStatePtr& state)
    {return context.getRandomGenerator()->sampleBool(k) ? policy1->selectAction(context, state) : policy2->selectAction(context, state);}

  virtual void observeTransition(ExecutionContext& context, const Variable& action, double reward, const DecisionProblemStatePtr& newState)
  {
    policy1->observeTransition(context, action, reward, newState);
    policy2->observeTransition(context, action, reward, newState);
  }

protected:
  friend class MixturePolicyClass;

  PolicyPtr policy1;
  PolicyPtr policy2;
  double k;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_PROBLEM_POLICY_H_
