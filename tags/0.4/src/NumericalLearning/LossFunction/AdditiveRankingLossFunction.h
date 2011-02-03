/*-----------------------------------------.---------------------------------.
| Filename: AdditiveRankingLossFunction.h  | Base class for additive         |
| Author  : Francis Maes                   |  ranking losses                 |
| Started : 25/10/2010 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_

# include <lbcpp/NumericalLearning/LossFunctions.h>

namespace lbcpp
{

class AdditiveRankingLossFunction : public RankingLossFunction
{
public:
  AdditiveRankingLossFunction(BinaryClassificationLossFunctionPtr baseLoss, const std::vector<double>& costs)
    : RankingLossFunction(costs), baseLoss(baseLoss) {}
  AdditiveRankingLossFunction() {}

  virtual bool isDerivable() const
    {return baseLoss->isDerivable();}

  const BinaryClassificationLossFunctionPtr& getBaseLoss() const
    {return baseLoss;}

protected:
  friend class AdditiveRankingLossFunctionClass;

  BinaryClassificationLossFunctionPtr baseLoss;

  void addRankingPair(double deltaCost, double deltaScore, size_t positiveAlternative, size_t negativeAlternative, double* output, std::vector<double>* gradient) const
  {
    jassert(deltaCost > 0);
    // deltaScore = scores[positiveAlternative] - scores[negativeAlternative]
    // deltaScore should be positive
    
    double discriminantValue, discriminantDerivative;
    baseLoss->computePositive(deltaScore, output ? &discriminantValue : NULL, NULL, gradient ? &discriminantDerivative : NULL);
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

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ADDITIVE_RANKING_H_
