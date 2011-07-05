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

  struct ProcessFoldWorkUnit : public WorkUnit
  {
    ProcessFoldWorkUnit(ContainerPtr trainingData, ContainerPtr testingData, const String& description)
      : trainingData(trainingData), testingData(testingData), description(description) {}

    virtual String toShortString() const
      {return description;} 

    virtual Variable run(ExecutionContext& context)
    {
      size_t numTrainingExamples = trainingData->getNumElements();
      FunctionPtr lossFunction = oneAgainstAllMultiClassLossFunction(hingeDiscriminativeLossFunction());
      EvaluatorPtr evaluator = defaultSupervisedEvaluator();

      std::vector<OnlineLearnerPtr> onlineLearners;
      onlineLearners.push_back(stochasticGDOnlineLearner(lossFunction, constantIterationFunction(1.0))); // lbcpp sgd "factory" setting
      onlineLearners.push_back(stochasticGDOnlineLearner(lossFunction, invLinearIterationFunction(2.0, numTrainingExamples))); // another good setting
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 0)); // infinite memory
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, numTrainingExamples / 5)); // "factory" setting
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 10));   // auto sgd with different memory sizes ...
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 100));
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 1000));
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 10000));
      onlineLearners.push_back(autoStochasticGDOnlineLearner(lossFunction, 100000));

      static const int maxIterations = 100;

      BatchLearnerPtr batchLearner = stochasticBatchLearner(maxIterations);
      for (size_t i = 0; i < onlineLearners.size(); ++i)
      {
        OnlineLearnerPtr sgdLearner = onlineLearners[i];
        context.enterScope(sgdLearner->toShortString());

        // create online learner
        std::vector<OnlineLearnerPtr> cl;
        cl.push_back(sgdLearner);
        cl.push_back(evaluatorOnlineLearner());
        cl.push_back(restoreBestParametersOnlineLearner());
        OnlineLearnerPtr onlineLearner = compositeOnlineLearner(cl);

        // create classifier
        FunctionPtr classifier = linearMultiClassClassifier(new SimpleLearnerParameters(batchLearner, onlineLearner));

        // train and evaluate
        classifier->train(context, trainingData, testingData, T("Training"));
        ScoreObjectPtr trainingScore = classifier->evaluate(context, trainingData, evaluator, T("Evaluating on training data"));
        ScoreObjectPtr testingScore = classifier->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));

        context.leaveScope(testingScore);
      }
      return true;
    }

  protected:
    ContainerPtr trainingData;
    ContainerPtr testingData;
    String description;
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

    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(T("Cross Validation"), numFolds);
    for (size_t i = 0; i < numFolds; ++i)
    {
      String description = T("Fold ") + String((int)i + 1) + T("/") + String((int)numFolds);
      workUnit->setWorkUnit(i, new ProcessFoldWorkUnit(data->invFold(i, numFolds), data->fold(i, numFolds), description));
    }
    context.run(workUnit);
    return Variable();
  }

protected:
  friend class AutoSGDSandBoxClass;

  File learningData;
  size_t maxCount;
  size_t numFolds;
};


}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_AUTO_SGD_SAND_BOX_H_
