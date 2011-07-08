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

  virtual void getInput(const ObjectPtr& example, SVector& result) const
    {convertDoubleVector(example->getVariable(0).getObjectAndCast<DoubleVector>(), result);}

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
        jassertfalse;
        return 0;
      }
    }
  }

  virtual size_t getNumClasses() const
    {jassert(labels); return labels->getNumElements();}

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
      return Variable::missingValue(getOutputType());

    getInput(inputs[0].getObject(), const_cast<LaRankClassifier*>(this)->configuration.testingData);
    const_cast<LaRankClassifier*>(this)->configuration.normTestingData = dot(configuration.testingData, configuration.testingData);
    
    std::vector<double> scores(getNumClasses());
    model->predict(scores);

    double scoreSum = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
      scoreSum += scores[i];
    
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    for (size_t i = 0; i < scores.size(); ++i)
      res->setValue(i, scores[i] / scoreSum);
    return res;
  }

protected:
  EnumerationPtr labels;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LARANK_CLASSIFIER_H_
