/*-----------------------------------------.---------------------------------.
| Filename: LinearRegressor.h              | Linear Regressors               |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2010 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LIBSVM_CLASSIFIER_H_
# define LBCPP_LEARNING_NUMERICAL_LIBSVM_CLASSIFIER_H_

# include "LibSVMLearningMachine.h"

namespace lbcpp
{

class LibSVMClassifier : public LibSVMLearningMachine
{
public:
  LibSVMClassifier(double C, LibSVMKernelType kernelType, size_t kernelDegree, double kernelGamma, double kernelCoef0)
    : LibSVMLearningMachine(C, kernelType, kernelDegree, kernelGamma, kernelCoef0) {}
  LibSVMClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(denseDoubleVectorClass(), enumValueType);}

  virtual size_t getSvmType() const
    {return C_SVC;}

  virtual svm_node* getInput(const ObjectPtr& example) const
    {return convertDoubleVector(example->getVariable(0).getObjectAndCast<DoubleVector>());}

  virtual double getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isEnumeration())
      return supervision.toDouble();
    else
    {
      DoubleVectorPtr vector = supervision.dynamicCast<DoubleVector>();
      if (vector)
        return (double)vector->getIndexOfMaximumValue();
      else
      {
        jassert(false);
        return 0.0;
      }
    }
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr supervisionType = inputVariables[1]->getType();
    if (supervisionType.isInstanceOf<Enumeration>())
      labels = supervisionType.staticCast<Enumeration>();
    else
    {
      labels = DoubleVector::getElementsEnumeration(supervisionType);
      jassert(labels);
    }
    return denseDoubleVectorClass(labels, probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!model)
      return Variable();

    DoubleVectorPtr input = inputs[0].getObjectAndCast<DoubleVector>();

    // predict probabilities
    std::vector<double> probs(model->nr_class);
    svm_node* node = convertDoubleVector(input);
    svm_predict_probability(model, &node[0], &probs[0]);
    delete [] node;

    // reorder classes and return probabilities
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    std::vector<int> labelIndices(model->nr_class);
    svm_get_labels(model, &labelIndices[0]);
    for (int i = 0; i < model->nr_class; ++i)
      res->setValue(labelIndices[i], probs[i]);
    return res;
  }

protected:
  EnumerationPtr labels;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LIBSVM_CLASSIFIER_H_
