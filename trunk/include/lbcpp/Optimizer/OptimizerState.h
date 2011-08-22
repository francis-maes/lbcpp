/*-----------------------------------------.---------------------------------.
| Filename: OptimizerState.h               | State associated with an        |
| Author  : Julien Becker                  | Optimizer (useful to restart    |
| Started : 22/08/2011 13:43               | the Optimizer)                  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_STATE_H_
# define LBCPP_OPTIMIZER_STATE_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class OptimizerState : public Object
{
public:
  OptimizerState(const Variable& initialParameters = Variable(), double initialScore = DBL_MAX)
    : bestParameters(initialParameters), bestScore(initialScore) {}

  const Variable& getBestParameters() const
    {return bestParameters;}

  void setBestParameters(const Variable& bestParameters)
    {this->bestParameters = bestParameters;}

  double getBestScore() const
    {return bestScore;}

  void setBestScore(double bestScore)
    {this->bestScore = bestScore;}

protected:
  friend class OptimizerStateClass;

  Variable bestParameters;
  double bestScore;
};

typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;

extern ClassPtr optimizerStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
