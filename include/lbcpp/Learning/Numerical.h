/*-----------------------------------------.---------------------------------.
| Filename: Numerical.h                    | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_H_
# define LBCPP_LEARNING_NUMERICAL_H_

# include "LearnableFunction.h"
# include "../Core/CompositeFunction.h"
# include "../Data/DoubleVector.h"
# include "../Function/StoppingCriterion.h"
# include "../Function/IterationFunction.h"
# include "../Function/Evaluator.h"
# include "../NumericalLearning/LossFunctions.h"

namespace lbcpp
{

class NumericalLearnableFunction : public LearnableFunction
{
public:
  const DoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DoubleVector>();}

  DoubleVectorPtr& getParameters()
    {return *(DoubleVectorPtr* )&parameters;}

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const DoubleVectorPtr& input, DoubleVectorPtr& target, double weight) const = 0;

  // returns false if no supervision is available
  virtual bool computeAndAddGradient(const FunctionPtr& lossFunction,
                                     const Variable* inputs, const Variable& output,
                                     double& exampleLossValue, DoubleVectorPtr& target, double weight) const = 0;

  // Function
  virtual String getOutputPostFix() const
    {return T("Prediction");}

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<NumericalLearnableFunction> NumericalLearnableFunctionPtr;

class StochasticGDParameters : public LearnerParameters
{
public:
  StochasticGDParameters( IterationFunctionPtr learningRate = constantIterationFunction(0.1),
                          StoppingCriterionPtr stoppingCriterion = maxIterationsWithoutImprovementStoppingCriterion(20),
                          size_t maxIterations = 100,
                          EvaluatorPtr evaluator = EvaluatorPtr(),
                          bool doPerEpisodeUpdates = false,
                          bool normalizeLearningRate = true,
                          bool restoreBestParameters = true,
                          bool randomizeExamples = true);

  virtual BatchLearnerPtr createBatchLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, const TypePtr& outputType) const;
  virtual OnlineLearnerPtr createOnlineLearner(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, const TypePtr& outputType) const;

  /*
  ** Learning rate
  */
  const IterationFunctionPtr& getLearningRate() const
    {return learningRate;}

  bool doNormalizeLearningRate() const
    {return normalizeLearningRate;}

  /*
  ** Max Iterations
  */
  size_t getMaxIterations() const
    {return maxIterations;}

  void setMaxIterations(size_t maxIterations)
    {this->maxIterations = maxIterations;}

  /*
  ** Stopping Criterion
  */
  const StoppingCriterionPtr& getStoppingCriterion() const
    {return stoppingCriterion;}

  void setStoppingCriterion(StoppingCriterionPtr stoppingCriterion)
    {this->stoppingCriterion = stoppingCriterion;}

  /*
  ** Loss Function
  */
  const FunctionPtr& getLossFunction() const
    {return lossFunction;}

  void setLossFunction(const FunctionPtr& function)
    {lossFunction = function;}

  /*
  ** Evaluator
  */
  const EvaluatorPtr& getEvaluator() const
    {return evaluator;}

  void setEvaluator(EvaluatorPtr evaluator)
    {this->evaluator = evaluator;}

  /*
  ** Restore Best Parameters
  */
  bool doRestoreBestParameters() const
    {return restoreBestParameters;}

  void setRestoreBestParameters(bool enabled)
    {restoreBestParameters = enabled;}

protected:
  friend class StochasticGDParametersClass;

  FunctionPtr lossFunction;
  IterationFunctionPtr learningRate;
  StoppingCriterionPtr stoppingCriterion;
  size_t maxIterations;
  EvaluatorPtr evaluator;
  bool doPerEpisodeUpdates;
  bool normalizeLearningRate;
  bool restoreBestParameters;
  bool randomizeExamples;
  // todo: regularizer
};

typedef ReferenceCountedObjectPtr<StochasticGDParameters> StochasticGDParametersPtr;

class SupervisedNumericalFunction : public CompositeFunction
{
public:
  SupervisedNumericalFunction(LearnerParametersPtr learnerParameters = LearnerParametersPtr())
    : learnerParameters(learnerParameters) {}

  virtual TypePtr getSupervisionType() const = 0;
  virtual FunctionPtr createLearnableFunction() const = 0;
  virtual void buildPostProcessing(CompositeFunctionBuilder& builder, size_t predictionIndex, size_t supervisionIndex) {}

  // Function
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

  virtual void buildFunction(CompositeFunctionBuilder& builder);

protected:
  friend class SupervisedNumericalFunctionClass;

  LearnerParametersPtr learnerParameters;
};

typedef ReferenceCountedObjectPtr<SupervisedNumericalFunction> SupervisedNumericalFunctionPtr;

// atomic learnable functions
extern LearnableFunctionPtr addBiasLearnableFunction(BinaryClassificationScore scoreToOptimize, double initialBias = 0.0);
extern NumericalLearnableFunctionPtr linearLearnableFunction();
extern NumericalLearnableFunctionPtr multiLinearLearnableFunction();

// batch learners
extern BatchLearnerPtr addBiasBatchLearner(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);

// online learners
extern OnlineLearnerPtr stochasticGDOnlineLearner(FunctionPtr lossFunction, IterationFunctionPtr learningRate, bool normalizeLearningRate = true);
extern OnlineLearnerPtr perEpisodeGDOnlineLearner(FunctionPtr lossFunction, IterationFunctionPtr learningRate, bool normalizeLearningRate = true);

// high-level learning machines
extern SupervisedNumericalFunctionPtr linearRegressor(LearnerParametersPtr parameters);
extern SupervisedNumericalFunctionPtr linearBinaryClassifier(LearnerParametersPtr parameters, bool incorporateBias = false, BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);
extern SupervisedNumericalFunctionPtr linearMultiClassClassifier(LearnerParametersPtr parameters);
extern FunctionPtr linearLearningMachine(LearnerParametersPtr parameters);


// conversion utilities
extern bool convertSupervisionVariableToBoolean(const Variable& supervision, bool& result);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_H_
