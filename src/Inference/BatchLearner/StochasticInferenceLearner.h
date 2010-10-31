/*-----------------------------------------.---------------------------------.
| Filename: StochasticInferenceLearner.h   | A batch learner that will call  |
| Author  : Francis Maes                   | online learner callbacks        |
| Started : 26/05/2010 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_

# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/InferenceStack.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Data/Pair.h>

namespace lbcpp
{

class StochasticPassInferenceLearner : public AtomicInferenceLearner
{
public:
  StochasticPassInferenceLearner(const std::vector<InferencePtr>& learnedInferences, bool randomizeExamples)
    : learnedInferences(learnedInferences), randomizeExamples(randomizeExamples)
    {}

  StochasticPassInferenceLearner() {}

  virtual TypePtr getOutputType(TypePtr ) const
    {return booleanType;}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

protected:
  friend class StochasticPassInferenceLearnerClass;

  std::vector<InferencePtr> learnedInferences;
  bool randomizeExamples;

  struct Callback : public InferenceCallback
  {
    Callback(InferencePtr targetInference) : targetInference(targetInference) {}

    virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
    {
      const InferencePtr& inference = stack->getCurrentInference();
      const InferenceOnlineLearnerPtr& onlineLearner = inference->getOnlineLearner();
      if (!onlineLearner || onlineLearner->isLearningStopped() || !supervision.exists())
        return;

      // call stepFinishedCallback
      onlineLearner->stepFinishedCallback(inference, input, supervision, output);

      // call subStepFinishedCallback
      for (int i = (int)stack->getDepth() - 2; i >= 0; --i)
      {
        const InferencePtr& parentInference = stack->getInference(i);
        const InferenceOnlineLearnerPtr& parentLearner = parentInference->getOnlineLearner();
        if (parentLearner && !parentLearner->isLearningStopped())
          parentLearner->subStepFinishedCallback(inference, input, supervision, output);
        if (parentInference == targetInference)
          break;
      }
    }

  private:
    InferencePtr targetInference;
  };

  virtual Variable learn(InferenceContextWeakPtr context, const InferencePtr& targetInference, const ContainerPtr& examples)
  {
    bool isDirectlyConnectedToOnlineLearner = (learnedInferences.size() == 1 && learnedInferences[0] == targetInference);
    const InferenceOnlineLearnerPtr& onlineLearner = targetInference->getOnlineLearner();
    jassert(!isDirectlyConnectedToOnlineLearner || onlineLearner);

    size_t n = examples->getNumElements();

    std::vector<size_t> order;
    if (randomizeExamples)
    {
      RandomGenerator::getInstance()->sampleOrder(n, order);
      jassert(n == order.size());
    }

    Callback callback(targetInference);
    callback.setStaticAllocationFlag();
    context->appendCallback(&callback);

    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = examples->getElement(randomizeExamples ? order[i] : i).getObjectAndCast<Pair>();
      if (isDirectlyConnectedToOnlineLearner)
      {
        // make a step
        onlineLearner->stepFinishedCallback(targetInference, example->getFirst(), example->getSecond(), Variable());
      }
      else
      {
        // make an episode
        Inference::ReturnCode returnCode = Inference::finishedReturnCode;
        context->run(targetInference,  example->getFirst(), example->getSecond(), returnCode);
        finishEpisode();
      }
    }

    context->removeCallback(&callback);

    if (isDirectlyConnectedToOnlineLearner)
    {
      // The "episode" concept is ill-defined in this case.
      // Since finishEpisode() should not be called too frequently, we prefer to call it once per pass instead of once per step
      finishEpisode();
    }

    return finishPass();
  }

  void finishEpisode()
  {
    for (size_t i = 0; i < learnedInferences.size(); ++i)
    {
      const InferencePtr& inference = learnedInferences[i];
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      if (learner && !learner->isLearningStopped())
        learner->episodeFinishedCallback(inference);
    }
  }

  bool finishPass() // returns false when learning is finished
  {
    bool wantsMoreIterations = false;
    for (size_t i = 0; i < learnedInferences.size(); ++i)
    {
      const InferencePtr& inference = learnedInferences[i];
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      if (!learner->isLearningStopped())
      {
        learner->passFinishedCallback(inference);
        wantsMoreIterations |= learner->wantsMoreIterations();
      }
    }
    return wantsMoreIterations;
  }
};

class StochasticInferenceLearner : public InferenceLearner<SequentialInference>
{
public:
  StochasticInferenceLearner(bool randomizeExamples = false)
    : randomizeExamples(randomizeExamples) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual InferencePtr createLearningPass(const InferencePtr& targetInference, ContainerPtr& trainingData)
  {
    // enumerate learners
    std::vector<InferencePtr> inferencesThatHaveALearner;
    targetInference->getInferencesThatHaveAnOnlineLearner(inferencesThatHaveALearner);
    jassert(inferencesThatHaveALearner.size());

    // call startLearningCallback()
    for (int i = (int)inferencesThatHaveALearner.size() - 1; i >= 0; --i)
      inferencesThatHaveALearner[i]->getOnlineLearner()->startLearningCallback();

    // create sequential inference state
    return new StochasticPassInferenceLearner(inferencesThatHaveALearner, randomizeExamples);
  }

  virtual SequentialInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    InferencePtr targetInference = getInference(input);
    ContainerPtr trainingData = getTrainingData(input);

    SequentialInferenceStatePtr res = new SequentialInferenceState(input, supervision);
    InferencePtr learningPass = createLearningPass(targetInference, trainingData);
    learningPass->setName(T("LearningPass ") + targetInference->getName());
    res->setSubInference(learningPass, Variable::pair(targetInference, trainingData), Variable());
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
    {return state->getSubOutput().getBoolean();} // repeat passes until a pass returns "false"

protected:
  friend class StochasticInferenceLearnerClass;

  bool randomizeExamples;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_
