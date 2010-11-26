/*-----------------------------------------.---------------------------------.
| Filename: StochasticInferenceLearner.h   | A batch learner that will call  |
| Author  : Francis Maes                   | online learner callbacks        |
| Started : 26/05/2010 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Inference/InferenceOnlineLearner.h>
# include <lbcpp/Inference/InferenceBatchLearner.h>
# include <lbcpp/Execution/ExecutionStack.h>

namespace lbcpp
{

class StochasticPassInferenceLearner : public InferenceBatchLearner<Inference>
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

    virtual void postInferenceCallback(ExecutionContext& context, const Variable& input, const Variable& supervision, Variable& output)
    {
      InferencePtr inference = context.getCurrentFunction().staticCast<Inference>();
      const InferenceOnlineLearnerPtr& onlineLearner = inference->getOnlineLearner();
      if (!onlineLearner || onlineLearner->isLearningStopped() || !supervision.exists())
        return;

      // call stepFinishedCallback
      onlineLearner->stepFinishedCallback(context, inference, input, supervision, output);

      // call subStepFinishedCallback
      for (int i = (int)context.getStackDepth() - 2; i >= 0; --i)
      {
        const InferencePtr& parentInference = context.getStack()->getFunction(i);
        const InferenceOnlineLearnerPtr& parentLearner = parentInference->getOnlineLearner();
        if (parentLearner && !parentLearner->isLearningStopped())
          parentLearner->subStepFinishedCallback(context, inference, input, supervision, output);
        if (parentInference == targetInference)
          break;
      }
    }

  private:
    InferencePtr targetInference;
  };

  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const InferencePtr& targetInference = learnerInput->getTargetInference();
    bool isDirectlyConnectedToOnlineLearner = (learnedInferences.size() == 1 && learnedInferences[0] == targetInference);
    const InferenceOnlineLearnerPtr& onlineLearner = targetInference->getOnlineLearner();
    jassert(!isDirectlyConnectedToOnlineLearner || onlineLearner);

    size_t n = learnerInput->getNumTrainingExamples();

    std::vector<size_t> order;
    if (randomizeExamples)
    {
      RandomGenerator::getInstance()->sampleOrder(n, order);
      jassert(n == order.size());
    }

    Callback callback(targetInference);
    callback.setStaticAllocationFlag();
    context.appendCallback(&callback);

    for (size_t i = 0; i < n; ++i)
    {
      const std::pair<Variable, Variable>& example = learnerInput->getTrainingExample(randomizeExamples ? order[i] : i);
      if (isDirectlyConnectedToOnlineLearner)
      {
        // make a step
        jassert(!example.second.isNil()); // use a missing value if there is no supervision
        onlineLearner->stepFinishedCallback(context, targetInference, example.first, example.second, Variable());
      }
      else
      {
        // make an episode
        if (!runInference(context, targetInference, example.first, example.second))
          return Variable();
        finishEpisode(context);
      }
    }

    context.removeCallback(&callback);

    if (isDirectlyConnectedToOnlineLearner)
    {
      // The "episode" concept is ill-defined in this case.
      // Since finishEpisode() should not be called too frequently, we prefer to call it once per pass instead of once per step
      finishEpisode(context);
    }

    return finishPass(context, learnerInput);
  }

  void finishEpisode(ExecutionContext& context) const
  {
    for (size_t i = 0; i < learnedInferences.size(); ++i)
    {
      const InferencePtr& inference = learnedInferences[i];
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      if (learner && !learner->isLearningStopped())
        learner->episodeFinishedCallback(context, inference);
    }
  }

  bool finishPass(ExecutionContext& context, const InferenceBatchLearnerInputPtr& learnerInput) const // returns false when learning is finished
  {
    bool wantsMoreIterations = false;
    for (size_t i = 0; i < learnedInferences.size(); ++i)
    {
      const InferencePtr& inference = learnedInferences[i];
      const InferenceOnlineLearnerPtr& learner = inference->getOnlineLearner();
      if (!learner->isLearningStopped())
      {
        learner->passFinishedCallback(context, inference, learnerInput);
        wantsMoreIterations |= learner->wantsMoreIterations();
      }
    }
    return wantsMoreIterations;
  }
};

class StochasticInferenceLearner : public InferenceBatchLearner<SequentialInference>
{
public:
  StochasticInferenceLearner(bool randomizeExamples = false)
    : randomizeExamples(randomizeExamples) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    const InferencePtr& targetInference = learnerInput->getTargetInference();

    SequentialInferenceStatePtr res = new SequentialInferenceState(input, supervision);
    InferencePtr learningPass = createLearningPass(context, learnerInput);
    learningPass->setName(T("LearningPass ") + targetInference->getName());
    res->setSubInference(learningPass, learnerInput, Variable());
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr state) const
    {return state->getSubOutput().getBoolean();} // repeat passes until a pass returns "false"

protected:
  friend class StochasticInferenceLearnerClass;

  bool randomizeExamples;

  InferencePtr createLearningPass(ExecutionContext& context, const InferenceBatchLearnerInputPtr& learnerInput) const
  {
    // enumerate learners
    std::vector<InferencePtr> inferencesThatHaveALearner;
    learnerInput->getTargetInference()->getInferencesThatHaveAnOnlineLearner(context, inferencesThatHaveALearner);
    jassert(inferencesThatHaveALearner.size());

    // call startLearningCallback()
    for (int i = (int)inferencesThatHaveALearner.size() - 1; i >= 0; --i)
      inferencesThatHaveALearner[i]->getOnlineLearner()->startLearningCallback(context);

    // create sequential inference state
    return new StochasticPassInferenceLearner(inferencesThatHaveALearner, randomizeExamples);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_
