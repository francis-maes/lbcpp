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

class NumericalLearnableFunction : public Function
{
public:
  const EnumerationPtr& getParametersEnumeration() const
    {return parametersEnumeration;}

  virtual DoubleVectorPtr createParameters() const
    {return new DenseDoubleVector(parametersEnumeration, doubleType);}

  DoubleVectorPtr getOrCreateParameters()
  {
    DoubleVectorPtr res = getParameters();
    if (!res)
    {
      res = createParameters();
      setParameters(res);
    }
    return res;
  }

  virtual DoubleVectorPtr getParameters() const = 0;
  virtual void setParameters(const DoubleVectorPtr& parameters) = 0;

  virtual void addGradient(const Variable& lossDerivativeOrGradient, const Variable* inputs, const DoubleVectorPtr& target, double weight) const = 0;
  virtual bool computeLoss(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction, double& lossValue, Variable& lossDerivativeOrGradient) const = 0;

  virtual double getInputsSize(const Variable* inputs) const = 0;

  // Function
  virtual String getOutputPostFix() const
    {return T("Prediction");}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class NumericalLearnableFunctionClass;

  EnumerationPtr parametersEnumeration;
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
                          bool evaluateAtEachIteration = true,
                          size_t numExamplesPerIteration = 0);

  virtual BatchLearnerPtr createBatchLearner(ExecutionContext& context) const;
  virtual OnlineLearnerPtr createOnlineLearner(ExecutionContext& context) const;

  /*
  ** Learning rate
  */
  const IterationFunctionPtr& getLearningRate() const
    {return learningRate;}

  bool doNormalizeLearningRate() const
    {return normalizeLearningRate;}

  bool doPerEpisodeUpdates() const
    {return perEpisodeUpdates;}

  /*
  ** Max Iterations
  */
  size_t getMaxIterations() const
    {return maxIterations;}

  void setMaxIterations(size_t maxIterations)
    {this->maxIterations = maxIterations;}

  void setNumExamplesPerIteration(size_t numExamplesPerIteration)
    {this->numExamplesPerIteration = numExamplesPerIteration;}

  size_t getNumExamplesPerIteration() const
    {return numExamplesPerIteration;}

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

  void setEvaluateAtEachIteration(bool enabled)
    {evaluateAtEachIteration = enabled;}

protected:
  friend class StochasticGDParametersClass;

  FunctionPtr lossFunction;
  IterationFunctionPtr learningRate;
  StoppingCriterionPtr stoppingCriterion;
  size_t maxIterations;
  bool perEpisodeUpdates;
  bool normalizeLearningRate;
  bool restoreBestParameters;
  bool randomizeExamples;
  bool evaluateAtEachIteration;
  size_t numExamplesPerIteration;
  // todo: regularizer
};

typedef ReferenceCountedObjectPtr<StochasticGDParameters> StochasticGDParametersPtr;

// atomic learnable functions
extern FunctionPtr addBiasLearnableFunction(BinaryClassificationScore scoreToOptimize, double initialBias = 0.0);
extern NumericalLearnableFunctionPtr linearLearnableFunction();
extern NumericalLearnableFunctionPtr multiLinearLearnableFunction(EnumerationPtr outputsEnumeration = EnumerationPtr());
extern NumericalLearnableFunctionPtr rankingLearnableFunction(NumericalLearnableFunctionPtr scoringFunction);

// batch learners
extern BatchLearnerPtr addBiasBatchLearner(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);

// online learners
extern OnlineLearnerPtr stochasticGDOnlineLearner(FunctionPtr lossFunction, IterationFunctionPtr learningRate, bool normalizeLearningRate = true);
extern OnlineLearnerPtr perEpisodeGDOnlineLearner(FunctionPtr lossFunction, IterationFunctionPtr learningRate, bool normalizeLearningRate = true);
extern OnlineLearnerPtr parallelPerEpisodeGDOnlineLearner(FunctionPtr lossFunction, IterationFunctionPtr learningRate, bool normalizeLearningRate = true);

extern OnlineLearnerPtr autoStochasticGDOnlineLearner(FunctionPtr lossFunction, size_t episodeLength = 100, size_t memorySize = 0); // memorySize = 0 means +oo

// high-level learning machines
extern FunctionPtr linearRegressor(LearnerParametersPtr parameters);
extern FunctionPtr linearBinaryClassifier(LearnerParametersPtr parameters, bool incorporateBias = false, BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore);
extern FunctionPtr linearMultiClassClassifier(LearnerParametersPtr parameters);
extern FunctionPtr linearRankingMachine(LearnerParametersPtr parameters);
extern FunctionPtr linearLearningMachine(LearnerParametersPtr parameters);

// conversion utilities
extern bool convertSupervisionVariableToBoolean(const Variable& supervision, bool& result);

// libsvm
enum LibSVMKernelType
{
  linearKernel = 0,
  polynomialKernel,
  rbfKernel,
  sigmoidKernel
};
  
extern FunctionPtr libSVMClassifier(double C = 0.1, LibSVMKernelType kernelType = linearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0);
extern FunctionPtr libSVMBinaryClassifier(double C = 0.1, LibSVMKernelType kernelType = linearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0);
extern FunctionPtr libSVMLearningMachine(double C = 0.1, LibSVMKernelType kernelType = linearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0);
extern BatchLearnerPtr libSVMBatchLearner();

enum LibLinearSolverType
{
  l2RegularizedLogisticRegression = 0,
  l2RegularizedL2LossDual,
  l2RegularizedL2Loss,
  l2RegularizedL1LossDual,
  crammerAndSinger,
  l1RegularizedL2Loss,
  l1RegularizedLogisticRegression,
  l2RegularizedLogisticRegressionDual
};

extern FunctionPtr libLinearClassifier(double C, LibLinearSolverType solverType);
extern FunctionPtr libLinearBinaryClassifier(double C, LibLinearSolverType solverType);
extern BatchLearnerPtr libLinearBatchLearner();

enum LaRankKernelType
{
  laRankLinearKernel = 0,
  laRankPolynomialKernel,
  laRankRBFKernel
};

extern FunctionPtr laRankClassifier(double C = 0.1, LaRankKernelType kernelType = laRankLinearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0);
extern FunctionPtr laRankBinaryClassifier(double C = 0.1, LaRankKernelType kernelType = laRankLinearKernel, size_t kernelDegree = 1, double kernelGamma = 0.1, double kernelCoef0 = 0.0);
extern BatchLearnerPtr laRankBatchLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_H_
