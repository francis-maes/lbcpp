/*-----------------------------------------.---------------------------------.
| Filename: StochasticInferenceLearner.h   | A batch learner that will call  |
| Author  : Francis Maes                   | online learner callbacks        |
| Started : 26/05/2010 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_INFERENCE_BATCH_LEARNER_STOCHASTIC_H_

# include <lbcpp/Core/Pair.h>
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
    : learnedInferences(learnedInferences), randomizeExamples(randomizeExamples), iteration(0)
    {}

  StochasticPassInferenceLearner() {}

  virtual TypePtr getOutputType(TypePtr ) const
    {return pairClass(booleanType, doubleType);}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual String getDescription(ExecutionContext& context, const Variable& input, const Variable& supervision) const
    {return getName();}

protected:
  friend class StochasticPassInferenceLearnerClass;

  std::vector<InferencePtr> learnedInferences;
  bool randomizeExamples;
  size_t iteration;

  struct Callback : public ExecutionCallback
  {
    Callback(InferencePtr targetInference) : targetInference(targetInference) {}

    virtual void postExecutionCallback(const ExecutionStackPtr& stack, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& output)
    {
      ExecutionContext& context = getContext();
      const InferenceOnlineLearnerPtr& onlineLearner = inference->getOnlineLearner();
      if (!onlineLearner || onlineLearner->isLearningStopped() || !supervision.exists())
        return;

      // call stepFinishedCallback
      onlineLearner->stepFinishedCallback(context, inference, input, supervision, output);

      // call subStepFinishedCallback
      for (int i = (int)stack->getDepth() - 1; i >= 0; --i)
      {
        const WorkUnitPtr& workUnit = stack->getWorkUnit(i);
        InferenceWorkUnitPtr inferenceWorkUnit = workUnit.dynamicCast<InferenceWorkUnit>();
        if (inferenceWorkUnit)
        {
          const InferencePtr& parentInference = inferenceWorkUnit->getInference();
          const InferenceOnlineLearnerPtr& parentLearner = parentInference->getOnlineLearner();
          if (parentLearner && !parentLearner->isLearningStopped())
            parentLearner->subStepFinishedCallback(context, inference, input, supervision, output);
          if (parentInference == targetInference)
            break;
        }
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
        targetInference->run(context, example.first, example.second);
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

    ++(const_cast<StochasticPassInferenceLearner* >(this)->iteration);
    bool wantMoreIterators = finishPass(context, learnerInput);
    return new Pair(wantMoreIterators, onlineLearner ? onlineLearner->getLastLearner()->getDefaultScore() : 0.0);
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
    context.resultCallback(T("iteration"), (int)iteration);

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
    // send results
    std::vector< std::pair<String, double> > scores;
    learnerInput->getTargetInference()->getLastOnlineLearner()->getScores(scores);
    for (size_t i = 0; i < scores.size(); ++i)
      context.resultCallback(scores[i].first, scores[i].second);
    return wantsMoreIterations;
  }
};

class StochasticInferenceLearner : public InferenceBatchLearner<SequentialInference>
{
public:
  StochasticInferenceLearner(bool randomizeExamples = false, size_t maxIterations = 0)
    : randomizeExamples(randomizeExamples), maxIterations(maxIterations) {}

  virtual ClassPtr getTargetInferenceClass() const
    {return inferenceClass;}

  virtual SequentialInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    const InferenceBatchLearnerInputPtr& learnerInput = input.getObjectAndCast<InferenceBatchLearnerInput>(context);
    //const InferencePtr& targetInference = learnerInput->getTargetInference();

    SequentialInferenceStatePtr res = new SequentialInferenceState(input, supervision);
    InferencePtr learningPass = createLearningPass(context, learnerInput);
    //learningPass->setPushIntoStackFlag(true);
    res->setSubInference(learningPass, learnerInput, Variable());
    updateInference(context, res);
    return res;
  }

  // returns false if the final state is reached
  virtual bool updateInference(ExecutionContext& context, SequentialInferenceStatePtr state) const
  {
    context.progressCallback(new ProgressionState((double)state->getStepNumber(), (double)maxIterations, T("Learning Iterations")));
    state->incrementStepNumber();
    if (maxIterations && state->getStepNumber() > (int)maxIterations)
      return false;

    state->getSubInference()->setName(T("Learning Iteration ") + String((int)state->getStepNumber()));
    PairPtr pair = state->getSubOutput().dynamicCast<Pair>();
    // repeat passes until a pass returns "false"
    return !pair || pair->getFirst().getBoolean();
  }

protected:
  friend class StochasticInferenceLearnerClass;

  bool randomizeExamples;
  size_t maxIterations;

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
