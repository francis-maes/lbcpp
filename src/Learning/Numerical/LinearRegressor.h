/*-----------------------------------------.---------------------------------.
| Filename: LinearRegressor.h              | Linear Regressors               |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2010 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
# define LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_

# include <lbcpp/Core/Frame.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/NumericalLearning/LossFunctions.h>
# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class LinearRegressor : public SupervisedNumericalFunction
{
public:
  LinearRegressor(LearnerParametersPtr learnerParameters = LearnerParametersPtr())
    : SupervisedNumericalFunction(learnerParameters) {}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : (TypePtr)doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

    frameClass = new FrameClass(T("LinearRegressor"));
    frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));
    frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));
    frameClass->addMemberOperator(context, createObjectFunction(squareLossFunctionClass), 1);
    FunctionPtr linearFunction = new LinearLearnableFunction();
    linearFunction->setOnlineLearner(learnerParameters->createOnlineLearner());
    frameClass->addMemberOperator(context, linearFunction, 0, 2);

    setBatchLearner(learnerParameters->createBatchLearner());
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
