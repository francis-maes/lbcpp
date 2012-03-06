/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterion.h            | Stopping Criterion              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Mar 6, 2012  4:46:06 PM        |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_STOPPINGCRITERION_STOPPINGCRITERION_H_
# define LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_STOPPINGCRITERION_STOPPINGCRITERION_H_

# include "precompiled.h"

namespace lbcpp
{

class OptimizationProblemStatePtr;

class GeneralOptimizerStoppingCriterion : public Object
{
public:
  virtual bool performNext(ExecutionContext& context, size_t iteration, const OptimizationProblemStatePtr& state, const OptimizationProblemStatePtr& bestState) const = 0;

protected:
  friend class GeneralOptimizerStoppingCriterionClass;
};

typedef ReferenceCountedObjectPtr<GeneralOptimizerStoppingCriterion> GeneralOptimizerStoppingCriterionPtr;

}
; /* namespace lbcpp */

#endif //! LBCPP_PROTEIN_ROSETTA_PROTEINOPTIMIZER_STOPPINGCRITERION_STOPPINGCRITERION_H_
