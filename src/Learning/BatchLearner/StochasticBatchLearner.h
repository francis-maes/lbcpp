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
# include "../OnlineLearner/CompositeOnlineLearner.h"

namespace lbcpp
{

class StochasticBatchLearner : public BatchLearner
{
public:
  StochasticBatchLearner(const std::vector<FunctionPtr>& functionsToLearn, size_t maxIterations, bool randomizeExamples)
    : functionsToLearn(functionsToLearn), maxIterations(maxIterations), randomizeExamples(randomizeExamples) {}
  StochasticBatchLearner(size_t maxIterations, bool randomizeExamples)
    : maxIterations(maxIterations), randomizeExamples(randomizeExamples) {}

  StochasticBatchLearner() {}

  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    // identify functions that have an online learner
    std::vector<FunctionPtr> functionsToLearn = this->functionsToLearn;
    if (functionsToLearn.empty())
    {
      // functions to learn are not explicitly declared,
      // all sub-functions that have an online learner will be learned
      getAllFunctionsThatHaveAnOnlineLearner(function, functionsToLearn);
    }

    // start learning
    CompositeOnlineLearnerPtr compositeOnlineLearner = new CompositeOnlineLearner();
    for (size_t i = 0; i < functionsToLearn.size(); ++i)
      compositeOnlineLearner->startLearningAndAddLearner(context, functionsToLearn[i], maxIterations, trainingData, validationData);

    // perform learning iterations
    double startTime = Time::getMillisecondCounterHiRes();
    for (size_t i = 0; compositeOnlineLearner->getNumLearners() && (!maxIterations || i < maxIterations); ++i)
    {
      context.enterScope(T("Learning Iteration ") + String((int)i + 1));
      context.resultCallback(T("Iteration"), i + 1);
      Variable learningIterationResult = doLearningIteration(context, i, function, trainingData, compositeOnlineLearner);
      context.resultCallback(T("Time"), Variable((Time::getMillisecondCounterHiRes() - startTime) / 1000.0, timeType));
      context.leaveScope(learningIterationResult);
      context.progressCallback(new ProgressionState(i + 1, maxIterations, T("Learning Iterations")));
    }

    // finish learning
    compositeOnlineLearner->finishLearning();
    return true;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class StochasticBatchLearnerClass;

  std::vector<FunctionPtr> functionsToLearn;
  size_t maxIterations;
  bool randomizeExamples;
  

  static void getAllFunctionsThatHaveAnOnlineLearner(const FunctionPtr& function, std::vector<FunctionPtr>& res)
  {
    std::set<ObjectPtr> objects;
    function->getAllChildObjects(objects);
    for (std::set<ObjectPtr>::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
      FunctionPtr fun = it->dynamicCast<Function>();
      if (fun && fun->hasOnlineLearner())
        res.push_back(fun);
    }
/*    
    if (function->hasOnlineLearner())
      res.push_back(function);
    
    // special case for composite functions
    CompositeFunctionPtr compositeFunction = function.dynamicCast<CompositeFunction>();
    if (compositeFunction)
    {
      for (size_t i = 0; i < compositeFunction->getNumSubFunctions(); ++i)
        getAllFunctionsThatHaveAnOnlineLearner(compositeFunction->getSubFunction(i), res);
      return;
    }

    size_t n = function->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      Variable v = function->getVariable(i);
      FunctionPtr subFunction = v.dynamicCast<Function>();
      if (subFunction)
      {
        getAllFunctionsThatHaveAnOnlineLearner(subFunction, res);
        continue;
      }

      ContainerPtr container = v.dynamicCast<Container>();
      if (container && container->getElementsType()->inheritsFrom(functionClass))
      {
        size_t numSubFunctions = container->getNumElements();
        for (size_t j = 0; j < numSubFunctions; ++j)
        {
          FunctionPtr subFunction = container->getElement(j).getObjectAndCast<Function>();
          if (subFunction)
            getAllFunctionsThatHaveAnOnlineLearner(subFunction, res);
        }
      }
    }*/
  }

  void doEpisode(ExecutionContext& context, const FunctionPtr& function, const ObjectPtr& inputs, const OnlineLearnerPtr& onlineLearner) const
  {
    onlineLearner->startEpisode(inputs);
    Variable output = function->computeWithInputsObject(context, inputs);
    onlineLearner->finishEpisode(inputs, output);
  }

  Variable doLearningIteration(ExecutionContext& context, size_t iteration, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, CompositeOnlineLearnerPtr compositeOnlineLearner) const
  {
    jassert(compositeOnlineLearner->getNumLearners());
    compositeOnlineLearner->startLearningIteration(iteration);

    if (randomizeExamples)
    {
      std::vector<size_t> order;
      RandomGenerator::getInstance()->sampleOrder(trainingData.size(), order);
      for (size_t i = 0; i < order.size(); ++i)
        doEpisode(context, function, trainingData[order[i]], compositeOnlineLearner);
    }
    else
      for (size_t i = 0; i < trainingData.size(); ++i)
        doEpisode(context, function, trainingData[i], compositeOnlineLearner);
    return compositeOnlineLearner->finishLearningIterationAndRemoveFinishedLearners(iteration);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_STOCHASTIC_H_
