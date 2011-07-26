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
# include <lbcpp/Learning/LossFunction.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include "../../Core/Function/MapContainerFunction.h"

namespace lbcpp
{

class SupervisedNumericalFunction : public CompositeFunction
{
public:
  SupervisedNumericalFunction(LearnerParametersPtr learnerParameters = LearnerParametersPtr())
    : learnerParameters(learnerParameters) {}

  virtual TypePtr getInputType() const = 0;
  virtual TypePtr getSupervisionType() const = 0;
  virtual NumericalLearnableFunctionPtr createLearnableFunction() const = 0;
  virtual void buildPostProcessing(CompositeFunctionBuilder& builder, size_t predictionIndex, size_t supervisionIndex) {}

  // Function
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : getInputType();}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(getInputType());
    size_t supervision = builder.addInput(anyType);

    if (!learnableFunction)
      learnableFunction = createLearnableFunction();
    size_t prediction = builder.addFunction(learnableFunction, input, supervision);
    
    // move evaluator
    if (evaluator)
    {
      learnableFunction->setEvaluator(evaluator);
      //evaluator = EvaluatorPtr();
    }
    // set learners
    learnableFunction->setOnlineLearner(learnerParameters->createOnlineLearner(builder.getContext()));
    learnableFunction->setBatchLearner(learnerParameters->createBatchLearner(builder.getContext()));

    buildPostProcessing(builder, prediction, supervision);
  }

protected:
  friend class SupervisedNumericalFunctionClass;

  LearnerParametersPtr learnerParameters;
  NumericalLearnableFunctionPtr learnableFunction;
};

typedef ReferenceCountedObjectPtr<SupervisedNumericalFunction> SupervisedNumericalFunctionPtr;

class LinearRegressor : public SupervisedNumericalFunction
{
public:
  LinearRegressor(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters)
  {
  }
  LinearRegressor() {}

  virtual TypePtr getInputType() const
    {return doubleVectorClass();}

  virtual TypePtr getSupervisionType() const
    {return doubleType;}

  virtual NumericalLearnableFunctionPtr createLearnableFunction() const
  {
    NumericalLearnableFunctionPtr res = linearLearnableFunction();
    res->setEvaluator(regressionEvaluator());
    return res;
  }
};

class LinearBinaryClassifier : public SupervisedNumericalFunction
{
public:
  LinearBinaryClassifier(LearnerParametersPtr learnerParameters, bool incorporateBias, BinaryClassificationScore scoreToOptimize)
    : SupervisedNumericalFunction(learnerParameters), incorporateBias(incorporateBias), scoreToOptimize(scoreToOptimize)
  {
  }

  LinearBinaryClassifier() {}

  virtual TypePtr getInputType() const
    {return doubleVectorClass();}

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual void buildPostProcessing(CompositeFunctionBuilder& builder, size_t predictionIndex, size_t supervisionIndex)
  {
    if (incorporateBias)
      predictionIndex = builder.addFunction(addBiasLearnableFunction(scoreToOptimize), predictionIndex, supervisionIndex);
    builder.addFunction(signedScalarToProbabilityFunction(), predictionIndex);
  }

  virtual NumericalLearnableFunctionPtr createLearnableFunction() const
  {
    NumericalLearnableFunctionPtr res = linearLearnableFunction();
    res->setEvaluator(binaryClassificationEvaluator()); // todo: connect with scoreToOptimize
    return res;
  }

protected:
  friend class LinearBinaryClassifierClass;

  bool incorporateBias;
  BinaryClassificationScore scoreToOptimize;
};

class MultiClassScoresToDistributionFunction : public SimpleUnaryFunction
{
public:
  MultiClassScoresToDistributionFunction() : SimpleUnaryFunction(denseDoubleVectorClass(), denseDoubleVectorClass()) {}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    classes = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    jassert(classes);
    outputName = T("probabilities");
    outputShortName = T("probs");
    return denseDoubleVectorClass(classes, probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DenseDoubleVectorPtr& scores = input.getObjectAndCast<DenseDoubleVector>();
    if (!scores)
      return Variable::missingValue(getOutputType());

    jassert(classes);
    jassert(scores->getNumElements() == classes->getNumElements());
    size_t n = scores->getNumElements();

    static const double temperature = 1.0;

    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType(), n);
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      double score = scores->getValue(i);
      double prob = 1.0 / (1.0 + exp(-score * temperature));
      sum += prob;
      res->setValue(i, prob);
    }
    if (sum)
      res->multiplyByScalar(1.0 / sum);
    return res;
  }

protected:
  EnumerationPtr classes;
};

class LinearMultiClassClassifier : public SupervisedNumericalFunction
{
public:
  LinearMultiClassClassifier(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}

  LinearMultiClassClassifier() {}

  virtual TypePtr getInputType() const
    {return doubleVectorClass();}

  virtual TypePtr getSupervisionType() const
    {return anyType;} // enumValue or doubleVector[enumValue]

  virtual void buildPostProcessing(CompositeFunctionBuilder& builder, size_t predictionIndex, size_t supervisionIndex)
  {
    FunctionPtr scoresToProbabilities = new MultiClassScoresToDistributionFunction();
    builder.addFunction(scoresToProbabilities, predictionIndex);
    scoresToProbabilities->setBatchLearner(BatchLearnerPtr());
  }

  virtual NumericalLearnableFunctionPtr createLearnableFunction() const
  {
    NumericalLearnableFunctionPtr res = multiLinearLearnableFunction();
    res->setEvaluator(classificationEvaluator());
    return res;
  }
};

class LinearRankingMachine : public SupervisedNumericalFunction
{
public:
  LinearRankingMachine(LearnerParametersPtr learnerParameters)
    : SupervisedNumericalFunction(learnerParameters) {}
  LinearRankingMachine() {}

  virtual TypePtr getInputType() const
    {return containerClass(doubleVectorClass());}

  virtual TypePtr getSupervisionType() const
    {return doubleVectorClass(positiveIntegerEnumerationEnumeration);}

  virtual void buildPostProcessing(CompositeFunctionBuilder& builder, size_t predictionIndex, size_t supervisionIndex)
  {
    // todo: converter to probabilities

    /*FunctionPtr scoresToProbabilities = new MultiClassScoresToDistributionFunction();
    builder.addFunction(scoresToProbabilities, predictionIndex);
    scoresToProbabilities->setBatchLearner(BatchLearnerPtr());*/
  }

  virtual NumericalLearnableFunctionPtr createLearnableFunction() const
  {
    NumericalLearnableFunctionPtr res = rankingLearnableFunction(linearLearnableFunction());
    res->setEvaluator(rankingEvaluator());
    return res;
  }
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
    else if (supervisionType->inheritsFrom(enumValueType) || supervisionType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return linearMultiClassClassifier(learnerParameters);
    else if (supervisionType->inheritsFrom(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration)))
      return linearRankingMachine(learnerParameters);
    else
      return FunctionPtr();
  }

protected:
  friend class LinearLearningMachineClass;

  LearnerParametersPtr learnerParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_LINEAR_REGRESSOR_H_
