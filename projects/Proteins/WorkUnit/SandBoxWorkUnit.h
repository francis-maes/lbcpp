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

class EvaluateOnlineLearnerObjectiveFunction : public ObjectiveFunction
{
public:
  EvaluateOnlineLearnerObjectiveFunction(const InferencePtr& inferenceLearner, const InferenceBatchLearnerInputPtr& learnerInput)
    : inferenceLearner(inferenceLearner), learnerInput(learnerInput) {}
  EvaluateOnlineLearnerObjectiveFunction() {}

  virtual void customizeLearner(ExecutionContext& context, const Variable& input, const InferenceOnlineLearnerPtr& onlineLearner) const = 0;

  virtual double compute(ExecutionContext& context, const Variable& input) const
  {
    InferencePtr targetInference = learnerInput->getTargetInference()->cloneAndCast<Inference>(context);
    const InferenceOnlineLearnerPtr& onlineLearner = targetInference->getOnlineLearner();
    customizeLearner(context, input, onlineLearner);
    inferenceLearner->run(context, new InferenceBatchLearnerInput(targetInference, learnerInput->getTrainingExamples(), learnerInput->getValidationExamples()), Variable());
    return onlineLearner->getLastLearner()->getDefaultScore();
  }

protected:
  friend class EvaluateOnlineLearnerObjectiveFunctionClass;

  InferencePtr inferenceLearner;
  InferenceBatchLearnerInputPtr learnerInput;
};

typedef ReferenceCountedObjectPtr<EvaluateOnlineLearnerObjectiveFunction> EvaluateOnlineLearnerObjectiveFunctionPtr;

class EvaluateLearningRateObjectiveFunction : public EvaluateOnlineLearnerObjectiveFunction
{
public:
  EvaluateLearningRateObjectiveFunction(const InferencePtr& inferenceLearner, const InferenceBatchLearnerInputPtr& learnerInput)
    : EvaluateOnlineLearnerObjectiveFunction(inferenceLearner, learnerInput) {}
  EvaluateLearningRateObjectiveFunction() {}

  virtual TypePtr getInputType() const
    {return doubleType;}

  virtual String getDescription(const Variable& input) const
    {return T("Evaluating learning rate ") + String(input.getDouble());}

  virtual void customizeLearner(ExecutionContext& context, const Variable& input, const InferenceOnlineLearnerPtr& onlineLearner) const
  {
    int index = onlineLearner->getClass()->findObjectVariable(T("learningRate"));
    jassert(index >= 0);
    onlineLearner->setVariable(context, index, constantIterationFunction(input.getDouble()));
  }
};

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
    const EvaluateOnlineLearnerObjectiveFunctionPtr& objective = input.getObjectAndCast<EvaluateOnlineLearnerObjectiveFunction>();

    CompositeWorkUnitPtr workUnits(new CompositeWorkUnit(T("Optimizer"), 5));
    std::vector<double> scores(workUnits->getNumWorkUnits());
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); ++i)
    {
      double learningRate = pow(10.0, (double)i / 10.0 - 3.0);
      workUnits->setWorkUnit(i, evaluateObjectiveFunctionWorkUnit(objective->getDescription(learningRate), objective, learningRate, scores[i]));
    }
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

class OptimizerInferenceLearner : public InferenceBatchLearner<Inference>
{
public:
  OptimizerInferenceLearner(InferencePtr optimizer, InferencePtr baseLearner)
    : optimizer(optimizer), baseLearner(baseLearner) {}
  OptimizerInferenceLearner() {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>();
    EvaluateOnlineLearnerObjectiveFunctionPtr objective = new EvaluateLearningRateObjectiveFunction(baseLearner, learnerInput);
    
    Variable optimizedValue;
    if (!optimizer->run(context, objective, Variable(), &optimizedValue))
      return Variable();
    
    InferencePtr targetInference = learnerInput->getTargetInference();
    const InferenceOnlineLearnerPtr& onlineLearner = targetInference->getOnlineLearner();
    objective->customizeLearner(context, optimizedValue, onlineLearner);
    baseLearner->run(context, new InferenceBatchLearnerInput(targetInference, learnerInput->getTrainingExamples(), learnerInput->getValidationExamples()), Variable());
    return Variable();
  }

protected:
  InferencePtr optimizer;
  InferencePtr baseLearner;
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
