/*-----------------------------------------.---------------------------------.
| Filename: AdditiveRankingLossFunction.h  | Base class for additive         |
| Author  : Francis Maes                   |  ranking losses                 |
| Started : 25/10/2010 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class AdditiveRankingLossFunction : public RankingLossFunction
{
public:
  AdditiveRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss = DiscriminativeLossFunctionPtr())
    : baseLoss(baseLoss) {}

  virtual bool isDerivable() const
    {return baseLoss->isDerivable();}

  const DiscriminativeLossFunctionPtr& getBaseLoss() const
    {return baseLoss;}

protected:
  friend class AdditiveRankingLossFunctionClass;

  DiscriminativeLossFunctionPtr baseLoss;

  void addRankingPair(double deltaCost, double deltaScore, size_t positiveAlternative, size_t negativeAlternative, double* output, std::vector<double>* gradient) const
  {
    jassert(deltaCost > 0);

    // deltaScore = scores[positiveAlternative] - scores[negativeAlternative]
    // deltaScore should be positive
    
    double discriminantValue, discriminantDerivative;
    baseLoss->computeDiscriminativeLoss(deltaScore, output ? &discriminantValue : NULL, gradient ? &discriminantDerivative : NULL);
    if (gradient)
    {
      double delta = deltaCost * discriminantDerivative;
      (*gradient)[positiveAlternative] += delta;
      (*gradient)[negativeAlternative] -= delta;
    }
    if (output)
      *output += deltaCost * discriminantValue;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_
