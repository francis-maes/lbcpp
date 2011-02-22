/*-----------------------------------------.---------------------------------.
| Filename: SequentialDecisionSystem.h     | Sequential Decision System      |
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

class SequentialDecisionSystem : public Object
{
public:
  SequentialDecisionSystem(const FunctionPtr& initialStateSampler, const FunctionPtr& transitionFunction, const FunctionPtr& rewardFunction);
  SequentialDecisionSystem() {}

  const FunctionPtr& getInitialStateSampler() const
    {return initialStateSampler;}

  const FunctionPtr& getTransitionFunction() const
    {return transitionFunction;}

  const FunctionPtr& getRewardFunction() const
    {return rewardFunction;}

  bool initialize(ExecutionContext& context);

private:
  friend class SequentialDecisionSystemClass;

  FunctionPtr initialStateSampler;
  FunctionPtr transitionFunction;
  FunctionPtr rewardFunction;
};

typedef ReferenceCountedObjectPtr<SequentialDecisionSystem> SequentialDecisionSystemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_SYSTEM_LINEAR_POINT_PHYSIC_H_