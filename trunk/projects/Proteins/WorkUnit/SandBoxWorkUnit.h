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
# include "../Frame/ProteinFrame.h"

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

    StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(100), maxIterationsWithoutImprovementStoppingCriterion(100));
    lastLearner = lastLearner->setNextLearner(oldStoppingCriterionOnlineLearner(stoppingCriterion, true)); // stopping criterion
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
  virtual Variable run(ExecutionContext& context);

  ContainerPtr loadProteins(ExecutionContext& context, const String& workUnitName, const File& inputDirectory, const File& supervisionDirectory);

private:
  void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone);
};

class ComputeProteinFeaturesSandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (!dataDirectory.exists())
    {
      context.errorCallback(T("Missing data directory"));
      return false;
    }

    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, dataDirectory);
    if (!proteins->getNumElements())
    {
      context.errorCallback(T("No proteins"));
      return false;
    }
    
    context.enterScope(T("Waiting"));
    Thread::sleep(10000);
    context.leaveScope(T("ok"));

    context.enterScope(T("Computing features"));
    ProteinFrameFactory factory;
    FrameClassPtr proteinFrameClass = factory.createProteinFrameClass(context);
    std::vector<FramePtr> proteinFrames(proteins->getNumElements());
    for (size_t i = 0; i < proteinFrames.size(); ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      proteinFrames[i] = factory.createFrame(protein);
      proteinFrames[i]->ensureAllVariablesAreComputed();
    }
    context.leaveScope(T("ok"));

    context.enterScope(T("Waiting"));
    Thread::sleep(10000);
    context.leaveScope(T("ok"));
    return true;
  }

protected:
  friend class ComputeProteinFeaturesSandBoxClass;

  File dataDirectory;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
