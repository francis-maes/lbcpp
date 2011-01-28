/*-----------------------------------------.---------------------------------.
| Filename: InferenceBatchLearner.cpp      | Inference Batch Learner         |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 23:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceBatchLearner.h>
using namespace lbcpp;

/*
** InferenceBatchLearnerInput
*/
static void convert(const ContainerPtr& examples, InferenceExampleVectorPtr& res)
{
  if (!examples)
    return;
  InferenceExampleVectorPtr examplesVector = examples.dynamicCast<InferenceExampleVector>();
  if (examplesVector)
  {
    res = examplesVector;
    return;
  }

  size_t n = examples->getNumElements();
  res = new InferenceExampleVector(n);
  for (size_t i = 0; i < n; ++i)
    res->setElement(i, examples->getElement(i));
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, const InferenceExampleVectorPtr& trainingExamples, const InferenceExampleVectorPtr& validationExamples)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference),
    trainingData(trainingExamples), validationData(validationExamples)
{
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, const ContainerPtr& trainingExamples, const ContainerPtr& validationExamples)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference)
{
  convert(trainingExamples, trainingData);
  convert(validationExamples, validationData);
}

InferenceBatchLearnerInput::InferenceBatchLearnerInput(const InferencePtr& targetInference, size_t numTrainingExamples, size_t numValidationExamples)
  : Object(inferenceBatchLearnerInputClass(targetInference->getClass())), targetInference(targetInference),
    trainingData(new InferenceExampleVector(numTrainingExamples)),
    validationData(new InferenceExampleVector(numValidationExamples))
{
}

void InferenceBatchLearnerInput::setTargetInference(const InferencePtr& targetInference)
{
  jassert(targetInference);
//  setThisClass(inferenceBatchLearnerInputClass(targetInference->getClass()));
  this->targetInference = targetInference;
}

size_t InferenceBatchLearnerInput::getNumExamples() const
  {return getNumTrainingExamples() + getNumValidationExamples();}

const std::pair<Variable, Variable>& InferenceBatchLearnerInput::getExample(size_t i) const
{
  if (i < trainingData->size())
    return trainingData->get(i);
  i -= trainingData->size();
  jassert(i < validationData->size());
  return validationData->get(i);
}

std::pair<Variable, Variable>& InferenceBatchLearnerInput::getExample(size_t i)
{
  if (i < trainingData->size())
    return trainingData->get(i);
  i -= trainingData->size();
  jassert(i < validationData->size());
  return validationData->get(i);
}

void InferenceBatchLearnerInput::setExample(size_t i, const Variable& input, const Variable& supervision)
{
  std::pair<Variable, Variable>& e = getExample(i);
  e.first = input;
  e.second = supervision;
}

String InferenceBatchLearnerInput::toShortString() const
{
  String res = targetInference->getName() + T(", ") + String((int)getNumTrainingExamples()) + T(" train ex.");
  size_t n = getNumValidationExamples();
  if (n)
    res += T(" ") + String((int)n) + T(" validation ex.");
  return res;
}

/*
** EvaluateBatchLearnerObjectiveFunction
*/
String EvaluateBatchLearnerObjectiveFunction::getDescription(const Variable& input) const
  {return T("Evaluating learning parameters ") + input.toShortString();}

double EvaluateBatchLearnerObjectiveFunction::compute(ExecutionContext& context, const Variable& parameters) const
{
  jassert(learnerInput);
  InferencePtr targetInference = learnerInput->getTargetInference()->cloneAndCast<Inference>(context);
  InferencePtr inferenceLearner = createLearner(context, parameters, targetInference);

  context.resultCallback(T("parameters"), parameters);

  inferenceLearner->run(context, new InferenceBatchLearnerInput(targetInference, learnerInput->getTrainingExamples(), learnerInput->getValidationExamples()), Variable());
  return getObjective(context, targetInference);
}

InferencePtr lbcpp::autoTuneStochasticInferenceLearner(const OptimizerPtr& optimizer, const DistributionPtr& aprioriDistribution, const Variable& initialGuess)
  {return autoTuneInferenceLearner(optimizer, new OptimizerInput(evaluateStochasticLearnerObjectiveFunction(), aprioriDistribution, initialGuess));}
