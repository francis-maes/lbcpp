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
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

class LinearRegressor : public SupervisedNumericalFunction
{
public:
  LinearRegressor(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}
  LinearRegressor() {}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual FunctionPtr createLearnableFunction() const
    {return linearLearnableFunction();}
};

class LinearBinaryClassifier : public SupervisedNumericalFunction
{
public:
  LinearBinaryClassifier(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}
  LinearBinaryClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return booleanType;}

  virtual FunctionPtr createPostProcessing() const
    {return signedScalarToProbabilityFunction();}

  virtual FunctionPtr createLearnableFunction() const
    {return linearLearnableFunction();}
};

class LinearMultiClassClassifier : public SupervisedNumericalFunction
{
public:
  LinearMultiClassClassifier(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}
  LinearMultiClassClassifier() {}

  virtual TypePtr getSupervisionType() const
    {return enumValueType;}

  virtual FunctionPtr createPostProcessing() const
    {return mapContainerFunction(signedScalarToProbabilityFunction());}

  virtual FunctionPtr createLearnableFunction() const
    {return multiLinearLearnableFunction();}
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
    else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(enumerationDistributionClass()))
      return linearMultiClassClassifier(learnerParameters);
    else
      return FunctionPtr();
  }

protected:
  friend class LinearLearningMachineClass;

  LearnerParametersPtr learnerParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
