/*-----------------------------------------.---------------------------------.
| Filename: FisherFilterLearnableFunction.h| Fisher Filter                   |
| Author  : Julien Becker                  |        Learnable Function       |
| Started : 16/11/2011 16:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_LEARNABLE_FUNCTION_H_

namespace lbcpp
{

class FisherFilterLearnableFunction : public Function
{
public:
  FisherFilterLearnableFunction(size_t numSelectedFeatures = 0)
    : numSelectedFeatures(numSelectedFeatures)
  {
    setBatchLearner(fisherFilterBatchLearner());
  }

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass(anyType, doubleType) : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("fisherFilter");
    outputShortName = T("f");
    if (!outputEnumeration)
    {
      outputEnumeration = new DefaultEnumeration(T("FisherFilter"));
      for (size_t i = 0; i < numSelectedFeatures; ++i)
        outputEnumeration->addElement(context, String((int)i));
    }

    jassert(numSelectedFeatures <= inputVariables[0]->getType()->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements());
    return doubleVectorClass(outputEnumeration, doubleType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DoubleVectorPtr data = inputs[0].getObjectAndCast<DoubleVector>();
    DenseDoubleVectorPtr res = new DenseDoubleVector(outputEnumeration, doubleType);
    for (size_t i = 0; i < numSelectedFeatures; ++i)
      res->setValue(i, data->getElement(featuresMap[i]).getDouble());
    return res;
  }

protected:
  friend class FisherFilterLearnableFunctionClass;
  friend class FisherFilterBatchLearner;

  DefaultEnumerationPtr outputEnumeration;
  size_t numSelectedFeatures;
  std::vector<size_t> featuresMap;
};

typedef ReferenceCountedObjectPtr<FisherFilterLearnableFunction> FisherFilterLearnableFunctionPtr;
extern ClassPtr fisherFilterLearnableFunctionClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_FISHER_FILTER_LEARNABLE_FUNCTION_H_
