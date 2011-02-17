/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllMultiClassLoss...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 16/10/2010 13:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

// 1/(n-1) * sum_{i | i != correctClass} binaryLoss(score_{correctClass} - score_i)
class OneAgainstAllMultiClassLossFunction : public MultiClassLossFunction
{
public:
  OneAgainstAllMultiClassLossFunction(const DiscriminativeLossFunctionPtr& binaryLoss)
    : binaryLoss(binaryLoss) {}
  OneAgainstAllMultiClassLossFunction() {}

  virtual bool isDerivable() const
    {return binaryLoss->isDerivable();}
  
  virtual void computeMultiClassLoss(const DenseDoubleVectorPtr& scores, size_t correctClass, size_t numClasses, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    double correctValue = scores ? scores->getValue(correctClass) : 0.0;

    jassert(!gradientTarget || (*gradientTarget)->getNumElements() == numClasses);
    double invZ = 1.0 / (numClasses - 1.0);
    gradientWeight *= invZ;

    double correctValueDerivative = 0.0;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double derivative;
        double deltaValue = correctValue - (scores ? scores->getValue(i) : 0.0);
        double out = 0.0;
        binaryLoss->computeDiscriminativeLoss(deltaValue, output ? &out : NULL,  gradientTarget ? &derivative : NULL);
        if (output)
          *output += out * invZ;
        if (gradientTarget)
        {
          correctValueDerivative += derivative;
          (*gradientTarget)->decrementValue(i, derivative * gradientWeight);
        }
      }

    if (gradientTarget)
      (*gradientTarget)->incrementValue(correctClass, correctValueDerivative * gradientWeight);
  }

protected:
  friend class OneAgainstAllMultiClassLossFunctionClass;

  DiscriminativeLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_
