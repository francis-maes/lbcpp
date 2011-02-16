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

class MakeRegressionLossFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual String getOutputPostFix() const
    {return T("Loss");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return scalarFunctionClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return squareLossFunction(inputs[0].getDouble());}
};

class LinearRegressor : public FrameBasedFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? doubleType : (TypePtr)doubleVectorClass();}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

    frameClass = new FrameClass(T("LinearRegressor"));
    frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));
    frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));
    frameClass->addMemberOperator(context, new MakeRegressionLossFunction(), 1);
    FunctionPtr linearFunction = new LinearLearnableFunction();

    linearFunction->setOnlineLearner(compositeOnlineLearner(
      stochasticGDOnlineLearner(constantIterationFunction(0.1)),
      evaluatorOnlineLearner(regressionErrorEvaluator(T("toto"))),
      stoppingCriterionOnlineLearner(maxIterationsWithoutImprovementStoppingCriterion(10)),
      restoreBestParametersOnlineLearner()));

    frameClass->addMemberOperator(context, linearFunction, 0, 2);

    setBatchLearner(stochasticBatchLearner());
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
