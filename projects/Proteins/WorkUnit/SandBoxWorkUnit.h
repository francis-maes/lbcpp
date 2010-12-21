/*-----------------------------------------.---------------------------------.
| Filename: SandBoxWorkUnit.h              | Sand Box Work Unit              |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 13:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
# define LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"
# include "../Perception/ProteinPerception.h"
# include "../Inference/ProteinInferenceFactory.h"
# include "../Inference/ProteinInference.h"
# include "../Inference/ContactMapInference.h"
# include "../Evaluator/ProteinEvaluator.h"

namespace lbcpp
{

class MyLearningParameters : public InferenceOnlineLearnerParameters
{
public:
  MyLearningParameters() : logLearningRate(1.0), logLearningRateDecrease(3.0), logRegularizer(-7.0) {}

  double getLogLearningRate() const
    {return logLearningRate;}

  double getLogLearningRateDecrease() const
    {return logLearningRateDecrease;}

  double getLogRegularizer() const
    {return logRegularizer;}

  virtual InferenceOnlineLearnerPtr createLearner() const
  {
    InferenceOnlineLearnerPtr res, lastLearner;
    res = lastLearner = gradientDescentOnlineLearner(
          perStep, invLinearIterationFunction(pow(10.0, logLearningRate), (size_t)(pow(10.0, logLearningRateDecrease))), true,
          perStepMiniBatch20, l2RegularizerFunction(pow(10.0, logRegularizer)));

    EvaluatorPtr evaluator = classificationAccuracyEvaluator();

    EvaluatorPtr trainEvaluator = evaluator->cloneAndCast<Evaluator>();
    trainEvaluator->setName(T("trainScore"));
    lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(trainEvaluator, false));

    EvaluatorPtr validationEvaluator = evaluator->cloneAndCast<Evaluator>();
    validationEvaluator->setName(T("validationScore"));
    lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(validationEvaluator, true));

    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(3);
    lastLearner = lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true)); // stopping criterion
    return res;
  }

protected:
  friend class MyLearningParametersClass;

  double logLearningRate;
  double logLearningRateDecrease;
  double logRegularizer;
};

typedef ReferenceCountedObjectPtr<MyLearningParameters> MyLearningParametersPtr;

extern ClassPtr myLearningParametersClass;

///////////////////////////////////////////////

// Optimizer: ObjectiveFunction x Aprioris -> Variable
// OptimizerInferenceLearner: decorates the optimizer

class AlaRacheOptimizer : public Inference
{
public:
  virtual TypePtr getInputType() const
    {return objectiveFunctionClass;}

  virtual TypePtr getOutputType(TypePtr input) const
    {return doubleType;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const ObjectiveFunctionPtr& objective = input.getObjectAndCast<ObjectiveFunction>();

    CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(T("Optimizer"), 5));
    std::vector<double> scores(workUnits->getNumWorkUnits());
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); ++i)
    {
      double learningRate = pow(10.0, (double)i / 10.0 - 3.0);
      workUnits->setWorkUnit(i, evaluateObjectiveFunctionWorkUnit(objective->getDescription(learningRate), objective, learningRate, scores[i]));
    }
    workUnits->setPushChildrenIntoStackFlag(true);
    context.run(workUnits);
    double bestScore = -DBL_MAX;
    double res = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      double learningRate = pow(10.0, (double)i / 10.0 - 3.0);
      std::cout << "Score for LR = " << learningRate << ": " << scores[i] << std::endl;
      if (scores[i] > bestScore)
        bestScore = scores[i], res = learningRate;
    }
    return res;
  }
};

class SandBoxWorkUnit : public WorkUnit
{
public:
  SandBoxWorkUnit() : WorkUnit(T("SandBox")) {}

  virtual bool run(ExecutionContext& context);

  VectorPtr loadProteins(ExecutionContext& context, const String& workUnitName, const File& inputDirectory, const File& supervisionDirectory);

private:
  void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
