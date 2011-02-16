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

class SupervisedLinearNumericalFunction : public SupervisedNumericalFunction
{
public:
  SupervisedLinearNumericalFunction(LearnerParametersPtr learnerParameters, ClassPtr lossFunctionClass)
    : SupervisedNumericalFunction(learnerParameters), lossFunctionClass(lossFunctionClass) {}
  SupervisedLinearNumericalFunction() {}

  virtual FunctionPtr createPostProcessing() const
    {return FunctionPtr();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());

    frameClass = new FrameClass(getClassName() + T("Frame"));
    frameClass->addMemberVariable(context, inputVariables[0]->getType(), T("input"));             // 0: input
    frameClass->addMemberVariable(context, inputVariables[1]->getType(), T("supervision"));       // 1: supervision
    frameClass->addMemberOperator(context, createObjectFunction(lossFunctionClass), 1);           // 2: loss(supervision)

    FunctionPtr linearFunction = new LinearLearnableFunction();
    linearFunction->setOnlineLearner(learnerParameters->createOnlineLearner());
    frameClass->addMemberOperator(context, linearFunction, 0, 2);                                 // 3: linearFunction(0,2)

    FunctionPtr postProcessing = createPostProcessing();
    if (postProcessing)
      frameClass->addMemberOperator(context, postProcessing, 3);                                  // 4: postProcess(3)          

    setBatchLearner(learnerParameters->createBatchLearner());
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }

protected:
  friend class SupervisedLinearNumericalFunctionClass;

  ClassPtr lossFunctionClass;
};

class LinearRegressor : public SupervisedLinearNumericalFunction
{
public:
  LinearRegressor(LearnerParametersPtr learnerParameters, ClassPtr lossFunctionClass)
    : SupervisedLinearNumericalFunction(learnerParameters, lossFunctionClass) {}
  LinearRegressor() {}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}
};

class SignedScalarToProbabilityFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}

  virtual String getOutputPostFix() const
    {return T("Prob");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!input.exists())
      return Variable::missingValue(probabilityType);

    double score = input.getDouble();
    static const double temperature = 1.0;
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType);
  }
};

class LinearBinaryClassifier : public SupervisedLinearNumericalFunction
{
public:
  LinearBinaryClassifier(LearnerParametersPtr learnerParameters, ClassPtr lossFunctionClass)
    : SupervisedLinearNumericalFunction(learnerParameters, lossFunctionClass) {}
  LinearBinaryClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return booleanType;}

  virtual FunctionPtr createPostProcessing() const
    {return new SignedScalarToProbabilityFunction();}
};

class LinearLearningMachine : public ProxyFunction
{
public:
  LinearLearningMachine(LearnerParametersPtr learnerParameters = new StochasticGDParameters())
    : learnerParameters(learnerParameters) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)doubleVectorClass() : anyType;}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputsType = inputVariables[0]->getType();
    TypePtr supervisionType = inputVariables[1]->getType();

    if (supervisionType == doubleType)
      return linearRegressor(learnerParameters);
    else if (supervisionType == probabilityType || supervisionType == booleanType)
      return linearBinaryClassifier(learnerParameters);
    else
      return FunctionPtr();
  }

protected:
  friend class LinearLearningMachineClass;

  LearnerParametersPtr learnerParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
