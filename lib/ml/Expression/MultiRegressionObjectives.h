/*-----------------------------------------.---------------------------------.
| Filename: MultiRegressionObjectives.h    | Multi-dimensional Regression    |
| Author  : Francis Maes                   |  Objectives                     |
| Started : 08/01/2013 17:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_EXPRESSION_MULIT_REGRESSION_OBJECTIVES_H_
# define ML_EXPRESSION_MULIT_REGRESSION_OBJECTIVES_H_

# include <ml/Objective.h>

namespace lbcpp
{

class MSEMultiRegressionObjective : public SupervisedLearningObjective
{
public:
  MSEMultiRegressionObjective(TablePtr data, VariableExpressionPtr supervision)
    {configure(data, supervision);}
  MSEMultiRegressionObjective() {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = DBL_MAX; best = 0.0;}

  virtual double evaluatePredictions(ExecutionContext& context, DataVectorPtr predictions)
  {
    OVectorPtr supervisions = getSupervisions().staticCast<OVector>();
    EnumerationPtr enumeration = DenseDoubleVector::getElementsEnumeration(supervisions->getElementsType());
    size_t numOutputs = enumeration->getNumElements();
    
    // compute mean absolute error
    double squaredError = 0.0;
    for (DataVector::const_iterator it = predictions->begin(); it != predictions->end(); ++it)
    {
      DenseDoubleVectorPtr vector = it.getRawObject().staticCast<DenseDoubleVector>();
      for (size_t i = 0; i < numOutputs; ++i)
      {
        double prediction = vector->getValue(i);
        if (prediction == DVector::missingValue || !isNumberValid(prediction))
          prediction = 0.0;
        double delta = supervisions->get(it.getIndex()) - prediction;
        squaredError += delta * delta;
      }
    }

    // mean squared error
    return squaredError / (double)(supervisions->getNumElements() * numOutputs);
  }
};

}; /* namespace lbcpp */

#endif // !ML_EXPRESSION_MULIT_REGRESSION_OBJECTIVES_H_
