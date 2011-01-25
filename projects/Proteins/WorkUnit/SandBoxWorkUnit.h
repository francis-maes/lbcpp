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
  MyLearningParameters() : logLearningRate(0.0), logLearningRateDecrease(8.0), logRegularizer(-8.0) {}

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

    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(10);
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

class SandBoxWorkUnit : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context);

  VectorPtr loadProteins(ExecutionContext& context, const String& workUnitName, const File& inputDirectory, const File& supervisionDirectory);

private:
  void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone);
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
