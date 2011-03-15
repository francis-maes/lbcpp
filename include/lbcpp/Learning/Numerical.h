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
  virtual bool computeLoss(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction, double& lossValue, Variable& lossDerivativeOrGradient) const = 0;

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
                          bool doPerEpisodeUpdates = false,
                          bool normalizeLearningRate = true,
                          bool restoreBestParameters = true,
                          bool randomizeExamples = true,
                          bool evaluateAtEachIteration = true);

  virtual BatchLearnerPtr createBatchLearner(ExecutionContext& context) const;
  virtual OnlineLearnerPtr createOnlineLearner(ExecutionContext& context) const;

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
  ** Restore Best Parameters
  */
  bool doRestoreBestParameters() const
    {return restoreBestParameters;}

  void setRestoreBestParameters(bool enabled)
    {restoreBestParameters = enabled;}

  /*
  ** Evaluation
  */
  bool doEvaluateAtEachIteration() const
    {return evaluateAtEachIteration;}

protected:
  friend class StochasticGDParametersClass;

  FunctionPtr lossFunction;
  IterationFunctionPtr learningRate;
  StoppingCriterionPtr stoppingCriterion;
  size_t maxIterations;
  bool doPerEpisodeUpdates;
  bool normalizeLearningRate;
  bool restoreBestParameters;
  bool randomizeExamples;
  bool evaluateAtEachIteration;
  // todo: regularizer
};

typedef ReferenceCountedObjectPtr<StochasticGDParameters> StochasticGDParametersPtr;

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
extern FunctionPtr linearRegressor(LearnerParametersPtr parameters);
extern FunctionPtr linearBinaryClassifier(LearnerParametersPtr parameters, bool incorporateBias = false, BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);
extern FunctionPtr linearMultiClassClassifier(LearnerParametersPtr parameters);
extern FunctionPtr linearRankingMachine(LearnerParametersPtr parameters);
extern FunctionPtr linearLearningMachine(LearnerParametersPtr parameters);


// conversion utilities
extern bool convertSupervisionVariableToBoolean(const Variable& supervision, bool& result);

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_H_
