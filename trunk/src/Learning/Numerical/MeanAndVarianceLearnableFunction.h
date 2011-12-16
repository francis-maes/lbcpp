/*-----------------------------------------.---------------------------------.
| Filename: MeanAndVarianceLearnableF...h  | Mean And Variance               |
| Author  : Julien Becker                  |        Learnable Function       |
| Started : 28/11/2011 19:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_LEARNABLE_FUNCTION_H_

namespace lbcpp
{

class MeanAndVarianceLearnableFunction : public Function
{
public:
  MeanAndVarianceLearnableFunction(bool useMean = false, bool useVariance = true)
    : useMean(useMean), useVariance(useVariance)
  {
    setBatchLearner(meanAndVarianceBatchLearner());
  }

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass(anyType, doubleType) : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("meanAndVariance");
    outputShortName = T("m&v");

    return denseDoubleVectorClass(inputVariables[0]->getType()->getTemplateArgument(0), inputVariables[0]->getType()->getTemplateArgument(1));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DenseDoubleVectorPtr res = inputs[0].getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    std::cout << "MeanAndVarianceLearnableFunction: " << std::endl;
    EnumerationPtr enumeration = res->getElementsEnumeration();
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      std::cout << enumeration->getElement(i)->toString() << std::endl;
    jassertfalse;
    if (useMean)
      for (size_t i = 0; i < res->getNumValues(); ++i)
        res->setValue(i, res->getValue(i) - means[i]);
    if (useVariance)
      for (size_t i = 0; i < res->getNumValues(); ++i)
        res->setValue(i, res->getValue(i) / standardDeviations[i]);
    return res;
  }

protected:
  friend class MeanAndVarianceLearnableFunctionClass;
  friend class MeanAndVarianceBatchLearner;

  bool useMean;
  bool useVariance;

  std::vector<double> means;
  std::vector<double> standardDeviations;
};

typedef ReferenceCountedObjectPtr<MeanAndVarianceLearnableFunction> MeanAndVarianceLearnableFunctionPtr;
extern ClassPtr meanAndVarianceLearnableFunctionClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MEAN_AND_VARIANCE_LEARNABLE_FUNCTION_H_
