/*-----------------------------------------.---------------------------------.
| Filename: OptimizerInput.h               | Optimizer Input                 |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 23:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_INPUT_H_
# define LBCPP_OPTIMIZER_INPUT_H_

# include "../Core/Variable.h"
# include "../Distribution/Distribution.h"
# include "../Function/ObjectiveFunction.h"

namespace lbcpp
{

class OptimizerInput : public Object
{
public:
  OptimizerInput(const ObjectiveFunctionPtr& objective, const DistributionPtr& aprioriDistribution, const Variable& initialGuess)
    : objective(objective), aprioriDistribution(aprioriDistribution), initialGuess(initialGuess) {}
  OptimizerInput() {}

  const ObjectiveFunctionPtr& getObjective() const
    {return objective;}

  const DistributionPtr& getAprioriDistribution() const
    {return aprioriDistribution;}

  const Variable& getInitialGuess() const
    {return initialGuess;}

protected:
  friend class OptimizerInputClass;

  ObjectiveFunctionPtr objective;
  DistributionPtr aprioriDistribution;
  Variable initialGuess;
};

typedef ReferenceCountedObjectPtr<OptimizerInput> OptimizerInputPtr;

extern ClassPtr optimizerInputClass;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_INPUT_H_
