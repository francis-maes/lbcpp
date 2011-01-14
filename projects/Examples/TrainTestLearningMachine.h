/*-----------------------------------------.---------------------------------.
| Filename: TrainTestLearningMachine.h     | Train and test                  |
| Author  : Francis Maes                   |  a learning machine             |
| Started : 14/01/2011 17:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_
# define LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_

# include <lbcpp/Core/Variable.h>
# include <lbcpp/Core/Container.h>
# include <lbcpp/Data/Stream.h>
# include <lbcpp/Inference/Inference.h>
# include <lbcpp/NumericalLearning/NumericalSupervisedInference.h>
# include <lbcpp/DecisionTree/DecisionTree.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

enum LearningProblemType
{
  binaryClassificationProblem,
  multiClassClassificationProblem,
  multiLabelClassificationProblem,
  undefinedLearningProblem,
};

class LearningProblem : public Object
{
public:
  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file) = 0;
  virtual InferencePtr createInference(ExecutionContext& context, const String& methodToUse) = 0;
  virtual EvaluatorPtr createEvaluator(ExecutionContext& context) = 0;
};
typedef ReferenceCountedObjectPtr<LearningProblem> LearningProblemPtr;

class MultiClassClassificationProblem : public LearningProblem
{
public:
  MultiClassClassificationProblem() : inputClass(new UnnamedDynamicClass(T("Features"))), outputLabels(new Enumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return classificationDataTextParser(context, file, inputClass, outputLabels);}

  virtual InferencePtr createInference(ExecutionContext& context, const String& methodToUse)
  {
    if (methodToUse == String::empty || methodToUse == T("linear"))
      return multiClassLinearSVMInference(T("Classifier"), PerceptionPtr(), outputLabels);
    else if (methodToUse == T("extratree"))
      return classificationExtraTreeInference(T("Classifier"), PerceptionPtr(), outputLabels, 100, 10, 0);

    context.errorCallback(T("Unknown learning method ") + methodToUse);
    return InferencePtr();
  }

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return classificationAccuracyEvaluator();}

protected:
  DynamicClassPtr inputClass;
  EnumerationPtr outputLabels;
};

class TrainTestLearningMachine : public WorkUnit
{
public:
  TrainTestLearningMachine() : learningProblemType(undefinedLearningProblem) {}

  virtual bool run(ExecutionContext& context)
  {
    // create learning problem
    if (learningProblemType == undefinedLearningProblem)
    {
      context.errorCallback(T("Undefined learning problem"));
      return false;
    }
    LearningProblemPtr problem = getLearningProblem(context, learningProblemType);
    if (!problem)
      return false;

    // load training data
    ContainerPtr trainingData = loadData(context, problem, trainingFile, T("training data"));
    if (!trainingData)
      return false;

    // load testing data
    ContainerPtr testingData = loadData(context, problem, testingFile, T("testing data"));
    if (!testingData)
      return false;

    // create learning machine
    InferencePtr inference = problem->createInference(context, methodToUse);
    if (!inference)
      return false;

    // train
    if (!inference->train(context, trainingData, ContainerPtr(), T("Training")))
      return false;

    // evaluate on training data
    if (!inference->evaluate(context, trainingData, problem->createEvaluator(context), T("Evaluate on training data")))
      return false;

    // evaluate on testing data
    if (!inference->evaluate(context, testingData, problem->createEvaluator(context), T("Evaluate on testing data")))
      return false;

    return true;
  }

protected:
  friend class TrainTestLearningMachineClass;

  LearningProblemType learningProblemType;
  String methodToUse;
  File trainingFile;
  File testingFile;

  LearningProblemPtr getLearningProblem(ExecutionContext& context, LearningProblemType learningProblemType) const
  {
    if (learningProblemType == multiClassClassificationProblem)
      return new MultiClassClassificationProblem();
    context.errorCallback(T("Could not create Learning Problem"));
    return LearningProblemPtr();
  }

  ContainerPtr loadData(ExecutionContext& context, LearningProblemPtr problem, const File& file, const String& dataName) const
  {
    if (!file.exists())
    {
      context.errorCallback(T("Missing ") + dataName + T(" file"));
      return ContainerPtr();
    }
    StreamPtr stream = problem->createDataParser(context, file);
    if (!stream)
      return ContainerPtr();
    ContainerPtr res = stream->load();
    if (!res)
      return ContainerPtr();
    if (res->getNumElements() == 0)
      context.warningCallback(T("No ") + dataName);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_
