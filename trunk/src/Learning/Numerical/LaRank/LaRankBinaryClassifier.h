/*-----------------------------------------.---------------------------------.
| Filename: LaRankBinaryClassifier.h       | LaRank Binary Classifier        |
| Author  : Becker Julien                  |                                 |
| Started : 23/06/2011 14:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LARANK_BINARY_CLASSIFIER_H_
# define LBCPP_LEARNING_NUMERICAL_LARANK_BINARY_CLASSIFIER_H_

# include "LaRankClassifier.h"

namespace lbcpp
{

class LaRankBinaryClassifier : public LaRankClassifier
{
public:
  LaRankBinaryClassifier(double C, LaRankKernelType kernelType, size_t kernelDegree, double kernelGamma, double kernelCoef0)
    : LaRankClassifier(C, kernelType, kernelDegree, kernelGamma, kernelCoef0) {}
  LaRankBinaryClassifier() {}
  
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}
  
  virtual int getSupervision(const ObjectPtr& example) const
  {
    Variable supervision = example->getVariable(1);
    if (supervision.isBoolean())
      return supervision.getBoolean() ? 1 : 0;
    return supervision.getDouble() > 0.5 ? 1 : 0;
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

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
    std::vector<int> labelIndices(model->nr_class);
    svm_get_labels(model, &labelIndices[0]);
    return probability(probs[labelIndices[1]]);
    */
    return probability(0.f);
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LARANK_BINARY_CLASSIFIER_H_
