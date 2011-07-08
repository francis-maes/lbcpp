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
  
  virtual size_t getNumClasses() const
    {return 2;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!model)
      return Variable::missingValue(getOutputType());

    convertDoubleVector(inputs[0].getObjectAndCast<DoubleVector>(context), const_cast<LaRankBinaryClassifier*>(this)->configuration.testingData);
    const_cast<LaRankBinaryClassifier*>(this)->configuration.normTestingData = dot(configuration.testingData, configuration.testingData);

    std::vector<double> scores(getNumClasses());
    model->predict(scores);

    return convertToProbability(scores[1]);
  }

protected:
  Variable convertToProbability(double score) const
  {
    static const double temperature = 1.0;
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType);
  }
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LARANK_BINARY_CLASSIFIER_H_
