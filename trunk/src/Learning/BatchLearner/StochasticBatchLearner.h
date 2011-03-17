/*-----------------------------------------.---------------------------------.
| Filename: StochasticBatchLearner.h       | Stochastic Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
# define LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomGenerator.h>
# include "../OnlineLearner/HierarchicalOnlineLearner.h"

namespace lbcpp
{

class StochasticBatchLearner : public BatchLearner
{
public:
  StochasticBatchLearner(size_t maxIterations, bool randomizeExamples, size_t numExamplesPerIteration)
    : maxIterations(maxIterations), randomizeExamples(randomizeExamples), numExamplesPerIteration(numExamplesPerIteration) {}
  StochasticBatchLearner() {}

  struct IndexStream
  {
    virtual ~IndexStream() {}
    virtual size_t next() = 0;
  };

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    // start learning
    CompositeOnlineLearnerPtr onlineLearner = hierarchicalOnlineLearner();
    bool ok = onlineLearner->startLearning(context, function, maxIterations, trainingData, validationData);

    RandomGeneratorPtr random = new RandomGenerator();
    IndexStream* indexStream = createIndexStream(random, trainingData.size());

    // perform learning iterations
    double startTime = Time::getMillisecondCounterHiRes();
    bool isLearningFinished = false;
    for (size_t i = 0; !isLearningFinished && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);
      isLearningFinished = doLearningIteration(context, i, function, trainingData, *indexStream, onlineLearner);
      context.resultCallback(T("Time"), Variable((Time::getMillisecondCounterHiRes() - startTime) / 1000.0, timeType));
      context.leaveScope(onlineLearner.staticCast<HierarchicalOnlineLearner>()->getLastLearningIterationResult());
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));
    }

    delete indexStream;

    // finish learning
    onlineLearner->finishLearning();
    return ok;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class StochasticBatchLearnerClass;

  size_t maxIterations;
  bool randomizeExamples;
  size_t numExamplesPerIteration;

  // return true if learning is finished
  bool doLearningIteration(ExecutionContext& context, size_t iteration, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, IndexStream& indexStream, OnlineLearnerPtr onlineLearner) const
  {
    onlineLearner->startLearningIteration(iteration);
 
    size_t n = (numExamplesPerIteration ? numExamplesPerIteration : trainingData.size());
    for (size_t i = 0; i < n; ++i)
      doEpisode(context, function, trainingData[indexStream.next()], onlineLearner);

    double objective = 0.0;
    return onlineLearner->finishLearningIteration(iteration, objective);
  }
  
  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const OnlineLearnerPtr& onlineLearner) const
  {
    if (!inputs)
      return; // empty episode
    onlineLearner->startEpisode(inputs);
    Variable output = function->computeWithInputsObject(context, inputs);
    onlineLearner->finishEpisode(inputs, output);
  }

protected:
  struct OrderedIndexStream : public IndexStream
  {
    OrderedIndexStream(size_t size) : position(0), size(size) {}

    size_t position;
    size_t size;

    virtual size_t next()
    {
      size_t res = position;
      ++position;
      if (position == size)
        position = 0;
      return res;
    }
  };

  struct RandomizedIndexStream : public IndexStream
  {
    RandomizedIndexStream(RandomGeneratorPtr random, size_t containerSize) : random(random), position(0)
    {
      random->sampleOrder(containerSize, order);
    }

    RandomGeneratorPtr random;
    size_t position;
    std::vector<size_t> order;

    virtual size_t next()
    {
      size_t res = position;
      ++position;
      if (position == order.size())
      {
        position = 0;
        random->sampleOrder(order.size(), order);
      }
      return order[res];
    }
  };

  IndexStream* createIndexStream(RandomGeneratorPtr random, size_t size) const
  {
    if (randomizeExamples)
      return new RandomizedIndexStream(random, size);
    else
      return new OrderedIndexStream(size);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
