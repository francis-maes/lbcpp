/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizers                      |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "OptimizerInput.h"

namespace lbcpp
{

class Optimizer : public Function
{
public:
  virtual TypePtr getInputType() const
    {return optimizerInputClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return variableType;}
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
