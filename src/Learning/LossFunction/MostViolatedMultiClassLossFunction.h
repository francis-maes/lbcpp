/*-----------------------------------------.---------------------------------.
| Filename: MostViolatedMultiClassLossF...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 17/10/2010 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_

# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

// max_i binaryLoss(score_{correctClass} - score_i)
class MostViolatedMultiClassLossFunction : public MultiClassLossFunction
{
public:
  MostViolatedMultiClassLossFunction(const DiscriminativeLossFunctionPtr& binaryLoss)
    : binaryLoss(binaryLoss) {}
  MostViolatedMultiClassLossFunction() {}

  virtual bool isDerivable() const
    {return false;}
  
  virtual void computeMultiClassLoss(const DenseDoubleVectorPtr& scores, size_t correctClass, size_t numClasses, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    double correctValue = scores ? scores->getValue(correctClass) : 0.0;

    double worstValue = -DBL_MAX;
    std::set<size_t> worstClasses;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double value = scores ? scores->getValue(i) : 0.0;
        if (value >= worstValue)
        {
          if (value > worstValue)
            worstClasses.clear();
          worstClasses.insert(i);
          worstValue = value;
        }
      }
    jassert(worstValue > -DBL_MAX && worstClasses.size());

    // if more than one class are equally wrong, average losses
    // this is especially usefull when initializing learning with null parameters
    double invZ = 1.0 / (double)worstClasses.size();
    for (std::set<size_t>::const_iterator it = worstClasses.begin(); it != worstClasses.end(); ++it)
    {
      double derivative;
      double lossOutput;
      binaryLoss->computeDiscriminativeLoss(correctValue - worstValue, output ? &lossOutput : NULL, gradientTarget ? &derivative : NULL);
      if (output)
        *output += lossOutput * invZ;
      if (gradientTarget)
      {
        derivative *= gradientWeight * invZ;
        (*gradientTarget)->incrementValue(correctClass, derivative);
        (*gradientTarget)->incrementValue(*it, -derivative);
      }
    }
  }

protected:
  friend class MostViolatedMultiClassLossFunctionClass;

  DiscriminativeLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_
