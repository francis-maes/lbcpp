/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllMultiClassLoss...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 16/10/2010 13:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_ONE_AGAINST_ALL_MULTI_CLASS_LOSS_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_ONE_AGAINST_ALL_MULTI_CLASS_LOSS_H_

# include <lbcpp/Function/ScalarObjectFunction.h>
# include <lbcpp/Perception/PerceptionMaths.h>

namespace lbcpp
{

// 1/(n-1) * sum_{i | i != correctClass} binaryLoss(score_{correctClass} - score_i)
class OneAgainstAllMultiClassLossFunction : public MultiClassLossFunction
{
public:
  OneAgainstAllMultiClassLossFunction(const BinaryClassificationLossFunctionPtr& binaryLoss, size_t correctClass)
    : MultiClassLossFunction(correctClass), binaryLoss(binaryLoss) {}
  OneAgainstAllMultiClassLossFunction() {}

  virtual String toString() const
    {return T("OneAgainstAll(") + binaryLoss->toString() + T(", ") + String((int)correctClass) + T(")");}

  virtual bool isDerivable() const
    {return binaryLoss->isDerivable();}
  
  virtual void compute(const std::vector<double>& input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const
  {
    size_t n = input.size();
    jassert(correctClass < n && n > 1);
    double correctValue = input[correctClass];
    if (output)
      *output = NULL;

    jassert(!gradientTarget || gradientTarget->size() == n);
    double invZ = 1.0 / (n - 1.0);
    gradientWeight *= invZ;

    double correctValueDerivative = 0.0;
    for (size_t i = 0; i < n; ++i)
      if (i != correctClass)
      {
        double derivative;
        binaryLoss->computePositive(correctValue - input[i], output,  NULL, gradientTarget ? &derivative : NULL);
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

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_ONE_AGAINST_ALL_MULTI_CLASS_LOSS_H_
