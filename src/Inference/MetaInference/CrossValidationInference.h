/*-----------------------------------------.---------------------------------.
| Filename: CrossValidationInference.h     | Cross Validation Inference      |
| Author  : Francis Maes                   |                                 |
| Started : 02/10/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_CROSS_VALIDATION_H_
# define LBCPP_INFERENCE_META_CROSS_VALIDATION_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

class CrossValidateStepInference : public Inference
{
public:
  CrossValidateStepInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel)
    : Inference(name), evaluator(evaluator), inferenceModel(inferenceModel) {}
  CrossValidateStepInference() {}

  virtual TypePtr getInputType() const
    {return containerClass(pairClass(inferenceModel->getInputType(), inferenceModel->getSupervisionType()));}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return nilType;}


protected:
  friend class CrossValidateStepInferenceClass;

  EvaluatorPtr evaluator;
  InferencePtr inferenceModel;

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>(context);
    const ContainerPtr& trainingData = pair->getFirst().getObjectAndCast<Container>(context);
    const ContainerPtr& evaluationData = pair->getSecond().getObjectAndCast<Container>(context);
    InferencePtr inference = inferenceModel->cloneAndCast<Inference>(context);
    jassert(trainingData && evaluationData && inference);
    if (!train(context, inference, trainingData, ContainerPtr()))
      return Variable();
    evaluate(context, inference, evaluationData, evaluator);
    return Variable();
  }
};

class CrossValidationInference : public SharedParallelInference
{
public:
  CrossValidationInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel, size_t numFolds)
    : SharedParallelInference(name, new CrossValidateStepInference(name + T(" step"), evaluator, inferenceModel)), inferenceModel(inferenceModel), numFolds(numFolds) {}
  CrossValidationInference() {}

  virtual TypePtr getInputType() const
    {return subInference->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return nilType;}

  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const ContainerPtr& trainingData = input.getObjectAndCast<Container>(context);
    return String((int)numFolds) + T("-Cross Validating ") + inferenceModel->getName() + T(" with ") +
      String((int)trainingData->getNumElements()) + T(" ") + trainingData->getElementsType()->getName() + T("s");
  }

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const ContainerPtr& trainingData = input.getObjectAndCast<Container>(context);

    ParallelInferenceStatePtr state(new ParallelInferenceState(input, supervision));
    state->reserve(numFolds);
    for (size_t i = 0; i < numFolds; ++i)
    {
      Variable fold = Variable::pair(trainingData->invFold(i, numFolds), trainingData->fold(i, numFolds));
      state->addSubInference(subInference, fold, supervision);
    }
    return state;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
    {return Variable();}

protected:
  friend class CrossValidationInferenceClass;

  InferencePtr inferenceModel;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_CROSS_VALIDATION_H_
