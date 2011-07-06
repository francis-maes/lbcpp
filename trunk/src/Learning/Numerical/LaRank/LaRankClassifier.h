/*-----------------------------------------.---------------------------------.
| Filename: LaRankClassifier.h             | LaRank Classifier               |
| Author  : Julien Becker                  |                                 |
| Started : 06/07/2011 20:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LARANK_CLASSIFIER_H_
# define LBCPP_LEARNING_NUMERICAL_LARANK_CLASSIFIER_H_

# include "LaRankLearningMachine.h"

namespace lbcpp
{

class LaRankClassifier : public LaRankLearningMachine
{
public:
  LaRankClassifier(double C, LaRankKernelType kernelType, size_t kernelDegree, double kernelGamma, double kernelCoef0)
    : LaRankLearningMachine(C, kernelType, kernelDegree, kernelGamma, kernelCoef0) {}
  LaRankClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return sumType(denseDoubleVectorClass(), enumValueType);}

  virtual size_t getInput(const ObjectPtr& example, SVector& result) const
    {return convertDoubleVector(example->getVariable(0).getObjectAndCast<DoubleVector>(), result);}

  virtual int getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isEnumeration())
      return supervision.getInteger();
    else
    {
      DoubleVectorPtr vector = supervision.dynamicCast<DoubleVector>();
      if (vector)
        return (int)vector->getIndexOfMaximumValue();
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

    /*
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
    */
  }

protected:
  EnumerationPtr labels;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LARANK_CLASSIFIER_H_
