/*-----------------------------------------.---------------------------------.
| Filename: MostViolatedMultiClassLossF...h| Transforms a binary loss into a |
| Author  : Francis Maes                   |  multi-class loss               |
| Started : 17/10/2010 21:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_

# include <lbcpp/NumericalLearning/LossFunctions.h>

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
    std::set<size_t> wrongClasses;
    for (size_t i = 0; i < numClasses; ++i)
      if (i != correctClass)
      {
        double value = input ? (*input)[i] : 0.0;
        if (value >= wrongValue)
        {
          if (value > wrongValue)
            wrongClasses.clear();
          wrongClasses.insert(i);
          wrongValue = value;
        }
      }
    jassert(wrongValue > -DBL_MAX && wrongClasses.size());

    // if more than one class are equally wrong, average losses
    // this is especially usefull when initializing learning with null parameters
    double invZ = 1.0 / (double)wrongClasses.size();
    if (output)
      *output = 0;
    for (std::set<size_t>::const_iterator it = wrongClasses.begin(); it != wrongClasses.end(); ++it)
    {
      double derivative;
      double lossOutput;
      binaryLoss->computePositive(correctValue - wrongValue, output ? &lossOutput : NULL, NULL, gradientTarget ? &derivative : NULL);
      if (output)
        *output += lossOutput * invZ;
      if (gradientTarget)
      {
        derivative *= gradientWeight * invZ;
        (*gradientTarget)[correctClass] += derivative;
        (*gradientTarget)[*it] -= derivative;
      }
    }
  }

protected:
  friend class MostViolatedMultiClassLossFunctionClass;

  BinaryClassificationLossFunctionPtr binaryLoss;
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTION_MOST_VIOLATED_MULTI_CLASS_H_
