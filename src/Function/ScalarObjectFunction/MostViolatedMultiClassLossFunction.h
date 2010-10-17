/*-----------------------------------------.---------------------------------.
| Filename: MostViolatedMultiClassLossF...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 17/10/2010 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_MOST_VIOLATED_MULTI_CLASS_LOSS_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_MOST_VIOLATED_MULTI_CLASS_LOSS_H_

# include <lbcpp/Function/ScalarObjectFunction.h>
# include <lbcpp/Perception/PerceptionMaths.h>

namespace lbcpp
{

// max_i binaryLoss(score_{correctClass} - score_i)
class MostViolatedMultiClassLossFunction : public MultiClassLossFunction
{
public:
  MostViolatedMultiClassLossFunction(const BinaryClassificationLossFunctionPtr& binaryLoss, EnumerationPtr classes, size_t correctClass)
    : MultiClassLossFunction(classes, correctClass), binaryLoss(binaryLoss) {}
  MostViolatedMultiClassLossFunction() {}

  virtual String toString() const
    {return T("MostViolatedClass(") + binaryLoss->toString() + T(", ") + String((int)correctClass) + T(")");}

  virtual bool isDerivable() const
    {return false;}
  
  virtual void compute(const std::vector<double>* input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const
  {
    size_t numClasses = classes->getNumElements();
    jassert(correctClass < numClasses && numClasses > 1);
    double correctValue = input ? (*input)[correctClass] : 0.0;

    double wrongValue = -DBL_MAX;
    size_t wrongClass = 0;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double value = input ? (*input)[i] : 0.0;
        if (value > wrongValue)
          wrongValue = value, wrongClass = i;
      }
    jassert(wrongValue > -DBL_MAX);

    double derivative;
    binaryLoss->computePositive(correctValue - wrongValue, output, NULL, gradientTarget ? &derivative : NULL);
    if (gradientTarget)
    {
      derivative *= gradientWeight;
      (*gradientTarget)[correctClass] += derivative;
      (*gradientTarget)[wrongClass] -= derivative;
    }
  }

protected:
  friend class MostViolatedMultiClassLossFunctionClass;

  BinaryClassificationLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_MOST_VIOLATED_MULTI_CLASS_LOSS_H_
