/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllMultiClassLoss...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 16/10/2010 13:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_

# include "AdditiveRankingLossFunction.h"

namespace lbcpp
{

// 1/(n-1) * sum_{i | i != correctClass} binaryLoss(score_{correctClass} - score_i)
class OneAgainstAllMultiClassLossFunction : public MultiClassLossFunction
{
public:
  OneAgainstAllMultiClassLossFunction(const BinaryClassificationLossFunctionPtr& binaryLoss, EnumerationPtr classes, size_t correctClass)
    : MultiClassLossFunction(classes, correctClass), binaryLoss(binaryLoss) {}
  OneAgainstAllMultiClassLossFunction() {}

  virtual String toString() const
    {return T("OneAgainstAll(") + binaryLoss->toString() + T(", ") + String((int)correctClass) + T(")");}

  virtual bool isDerivable() const
    {return binaryLoss->isDerivable();}
  
  virtual void computeMultiClassLossFunction(const std::vector<double>* input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const
  {
    size_t numClasses = classes->getNumElements();
    jassert(correctClass < numClasses && numClasses > 1);
    double correctValue = input ? (*input)[correctClass] : 0.0;
    if (output)
      *output = 0;

    jassert(!gradientTarget || gradientTarget->size() == numClasses);
    double invZ = 1.0 / (numClasses - 1.0);
    gradientWeight *= invZ;

    double correctValueDerivative = 0.0;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double derivative;
        double deltaValue = correctValue - (input ? (*input)[i] : 0.0);
        binaryLoss->computePositive(deltaValue, output,  NULL, gradientTarget ? &derivative : NULL);
        if (gradientTarget)
        {
          correctValueDerivative += derivative;
          (*gradientTarget)[i] -= derivative * gradientWeight;
        }
      }

    if (gradientTarget)
      (*gradientTarget)[correctClass] += correctValueDerivative * gradientWeight;
    if (output)
      *output *= invZ;
  }

protected:
  friend class OneAgainstAllMultiClassLossFunctionClass;

  BinaryClassificationLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_ONE_AGAINST_ALL_MULTI_CLASS_H_
