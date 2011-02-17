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
  OneAgainstAllMultiClassLossFunction(const BinaryClassificationLossFunctionPtr& binaryLoss)
    : binaryLoss(binaryLoss) {}
  OneAgainstAllMultiClassLossFunction() {}

  virtual bool isDerivable() const
    {return binaryLoss->isDerivable();}
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    size_t numClasses = getNumClasses();
    size_t correctClass = getCorrectClass(otherInputs);
    jassert(numClasses > 1);
    double correctValue = input ? input->getValue(correctClass) : 0.0;

    jassert(!gradientTarget || (*gradientTarget)->getNumElements() == numClasses);
    double invZ = 1.0 / (numClasses - 1.0);
    gradientWeight *= invZ;

    double correctValueDerivative = 0.0;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double derivative;
        double deltaValue = correctValue - (input ? input->getValue(i) : 0.0);
        double out = 0.0;
        binaryLoss->computePositive(deltaValue, output ? &out : NULL,  NULL, gradientTarget ? &derivative : NULL);
        if (output)
          *output += out * invZ;
        if (gradientTarget)
        {
          correctValueDerivative += derivative;
          (*gradientTarget)->incrementValue(i, -derivative * gradientWeight);
        }
      }

    if (gradientTarget)
      (*gradientTarget)->incrementValue(correctClass, correctValueDerivative * gradientWeight);
  }

protected:
  friend class OneAgainstAllMultiClassLossFunctionClass;

  BinaryClassificationLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_
