/*-----------------------------------------.---------------------------------.
| Filename: AutoSGDSandBox.h               | Auto-adaptative                 |
| Author  : Francis Maes                   |  Stochastic Gradient Descent    |
| Started : 05/07/2011 20:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_AUTO_SGD_SAND_BOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_AUTO_SGD_SAND_BOX_H_

# include <lbcpp/Core/Container.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/LossFunction.h>

namespace lbcpp
{

class AutoSGDSandBox : public WorkUnit
{
public:
  AutoSGDSandBox() : maxCount(0), numFolds(10) {}

  struct LearnerResults : public Object
  {
    LearnerResults() : Object(objectClass),
        trainingScore(new ScalarVariableStatistics(T("Training Score"))),
        testingScore(new ScalarVariableStatistics(T("Testing Score"))) {}

    typedef std::vector<ScalarVariableStatistics> StatisticsVector;
    ClassPtr resultsClass;
    std::vector<StatisticsVector> results;

    ScalarVariableStatisticsPtr trainingScore;
    ScalarVariableStatisticsPtr testingScore;
  };
  typedef ReferenceCountedObjectPtr<LearnerResults> LearnerResultsPtr;

  struct TestLearnerWorkUnit : public WorkUnit
  {
    TestLearnerWorkUnit(ContainerPtr data, size_t numFolds, OnlineLearnerPtr onlineLearner, const String& description, LearnerResultsPtr& result)
      : data(data), numFolds(numFolds), onlineLearner(onlineLearner), description(description), result(result) {}

    virtual String toShortString() const
      {return description;} 

    virtual Variable run(ExecutionContext& context)
    {
      result = new LearnerResults();
      for (size_t i = 0; i < numFolds; ++i)
      {
        String description = T("Fold ") + String((int)i + 1) + T("/") + String((int)numFolds);
        context.enterScope(description);
        Variable res = runFold(context, data->invFold(i, numFolds), data->fold(i, numFolds), result);
        context.leaveScope(res);
      }
      displayResults(context, result);
      return new Pair(result->trainingScore->getMean(), result->testingScore->getMean());
    }

    Variable displayResults(ExecutionContext& context, LearnerResultsPtr result)
    {
      context.enterScope(T("Results"));
      context.resultCallback(T("Training"), result->trainingScore);
      context.resultCallback(T("Testing"), result->testingScore);

      jassert(result->resultsClass->getNumMemberVariables() == result->results.size());
      for (size_t j = 0; j < result->results[0].size(); ++j)
      {
        context.enterScope(T("Iteration ") + String((int)j + 1));
        context.resultCallback(T("Iteration"), j + 1);
        for (size_t i = 0; i < result->results.size(); ++i)
        {
          double mean = j < result->results[i].size() ? result->results[i][j].getMean() : 0.0;
          double stddev = j < result->results[i].size() ? result->results[i][j].getStandardDeviation() : 0.0;
          String name = result->resultsClass->getMemberVariableName(i);
          context.resultCallback(name, mean);
          context.resultCallback(name + T(" stddev"), stddev);
        }
        context.leaveScope();
      }
      context.leaveScope();
      return Variable();
    }

    Variable runFold(ExecutionContext& context, ContainerPtr trainingData, ContainerPtr testingData, LearnerResultsPtr result)
    {
      EvaluatorPtr evaluator = defaultSupervisedEvaluator();

      static const int maxIterations = 100;

      BatchLearnerPtr batchLearner = stochasticBatchLearner(maxIterations);

      // create classifier
      FunctionPtr classifier = linearMultiClassClassifier(new SimpleLearnerParameters(batchLearner, onlineLearner));

      // train and fill trace
      ExecutionTracePtr trainingTrace = new ExecutionTrace(T("training"));
      ExecutionCallbackPtr makeTraceCallback = makeTraceExecutionCallback(trainingTrace);
      context.appendCallback(makeTraceCallback);
      classifier->train(context, trainingData, testingData, T("Training"));
      context.removeCallback(makeTraceCallback);

      // update results given trace
      if (!updateResults(context, result, trainingTrace))
        return false;

      // evaluate
      ScoreObjectPtr trainingScore = classifier->evaluate(context, trainingData, evaluator, T("Evaluating on training data"));
      result->trainingScore->push(trainingScore->getScoreToMinimize());
      ScoreObjectPtr testingScore = classifier->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));
      result->testingScore->push(trainingScore->getScoreToMinimize());

      return testingScore->getScoreToMinimize();
    }

    bool updateResults(ExecutionContext& context, LearnerResultsPtr result, ExecutionTracePtr trainingTrace)
    {
      ExecutionTraceNodePtr node = trainingTrace->getRootNode();
      node = node->findSubNode(T("Training"));
      if (!node)
      {
        context.errorCallback(T("Could not find 'Training' trace node"));
        return false;
      }
      node = node->findFirstNode(); // "Learning predictions with ... training examples ... 
      if (!node)
      {
        context.errorCallback(T("Could not find learning curve trace node"));
        return false;
      }
      std::vector<ExecutionTraceItemPtr> items = node->getSubItems();
      size_t currentIteration = 0;
      for (size_t i = 0; i < items.size(); ++i)  // for each "Learning Iteration ..." node
      {
        ExecutionTraceNodePtr subNode = items[i].dynamicCast<ExecutionTraceNode>();
        if (subNode && subNode->toShortString().startsWith(T("Learning Iteration")))  
        {
          ObjectPtr iterationResults = subNode->getResultsObject(context);
          result->resultsClass = iterationResults->getClass();

          size_t n = iterationResults->getNumVariables();
          result->results.resize(n);
          for (size_t resultVariable = 0; resultVariable < n; ++resultVariable)
          {
            Variable v = iterationResults->getVariable(resultVariable);
            LearnerResults::StatisticsVector& res = result->results[resultVariable];
            if (res.size() <= currentIteration)
              res.resize(currentIteration + 1);
            if (v.isConvertibleToDouble())
              res[currentIteration].push(v.toDouble());
          }
          ++currentIteration;
        }
      }
      return true;
    }

  protected:
    ContainerPtr data;
    size_t numFolds;
    OnlineLearnerPtr onlineLearner;
    String description;
    LearnerResultsPtr& result;
  };

  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr features = new DefaultEnumeration(T("Features"));
    DefaultEnumerationPtr labels = new DefaultEnumeration(T("Labels"));
    StreamPtr parser = classificationLibSVMDataParser(context, learningData, features, labels);
    VectorPtr data = parser->load(maxCount);

    context.informationCallback(String((int)data->getNumElements()) + T(" examples, ")
        + String((int)features->getNumElements()) + T(" features, ")
        + String((int)labels->getNumElements()) + T(" labels"));

    FunctionPtr lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());

    size_t numTrainingExamples = data->getNumElements();

    std::vector< std::pair<OnlineLearnerPtr, String> > onlineLearners;
    onlineLearners.push_back(std::make_pair(stochasticGDOnlineLearner(lossFunction, constantIterationFunction(1.0)), T("constant(1)"))); // lbcpp sgd "factory" setting
    onlineLearners.push_back(std::make_pair(stochasticGDOnlineLearner(lossFunction, invLinearIterationFunction(2.0, numTrainingExamples)), T("invLinear(2,N)"))); // another good setting
    onlineLearners.push_back(std::make_pair(autoStochasticGDOnlineLearner(lossFunction, 0), T("autoSgd(+oo)"))); // infinite memory
    onlineLearners.push_back(std::make_pair(autoStochasticGDOnlineLearner(lossFunction, numTrainingExamples / 5), T("autoSgd(N/5)"))); // infinite memory
/*    onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 10));   // auto sgd with different memory sizes ...
    onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 100));
    onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 1000));
    onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 10000));
    onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 100000));*/

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Learners"), onlineLearners.size());
    std::vector<LearnerResultsPtr> perLearnerResults(onlineLearners.size());
    for (size_t i = 0; i < onlineLearners.size(); ++i)
    {
        OnlineLearnerPtr sgdLearner = onlineLearners[i];

        std::vector<OnlineLearnerPtr> cl;
        cl.push_back(sgdLearner);
        cl.push_back(evaluatorOnlineLearner());
        cl.push_back(restoreBestParametersOnlineLearner());
        OnlineLearnerPtr onlineLearner = compositeOnlineLearner(cl);
        
        workUnit->setWorkUnit(i, new TestLearnerWorkUnit(data, numFolds, onlineLearner, sgdLearner->toShortString(), perLearnerResults[i]));
    }

    workUnit->setPushChildrenIntoStackFlag(true);
    workUnit->setProgressionUnit(T("Learners"));
    context.run(workUnit);

    displaySummaryResults(context, onlineLearners, perLearnerResults);
    return Variable();
  }

  void displaySummaryResults(ExecutionContext& context, const std::vector<OnlineLearnerPtr>& onlineLearners, const std::vector<LearnerResultsPtr>& perLearnerResults)
  {
    jassert(onlineLearners.size() == perLearnerResults.size());

    context.enterScope(T("Summary Results"));

    ClassPtr resultsClass = perLearnerResults[0]->resultsClass;
    for (size_t i = 0; i < resultsClass->getNumMemberVariables(); ++i)
    {
      context.enterScope(resultsClass->getMemberVariableName(i));
      
      static const size_t maxIterations = 100;

      for (size_t iteration = 0; iteration < maxIterations; ++iteration)
      {
        context.enterScope(String((int)iteration + 1));
        context.resultCallback(T("Iteration"), iteration + 1);
        for (size_t learner = 0; learner < perLearnerResults.size(); ++learner)
        {
          LearnerResultsPtr learnerResults = perLearnerResults[learner];
          const LearnerResults::StatisticsVector& v = learnerResults->results[i];
          double mean = iteration < v.size() ? v[iteration].getMean() : 0.0;
          double stddev = iteration < v.size() ? v[iteration].getStandardDeviation() : 0.0;
          String name = onlineLearners[learner]->toShortString();
          context.resultCallback(name, mean);
          context.resultCallback(name + T(" Stddev"), stddev);
        }
        context.leaveScope();
      }
      context.leaveScope();
    }
    context.leaveScope();
  }

protected:
  friend class AutoSGDSandBoxClass;

  File learningData;
  size_t maxCount;
  size_t numFolds;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_AUTO_SGD_SAND_BOX_H_
