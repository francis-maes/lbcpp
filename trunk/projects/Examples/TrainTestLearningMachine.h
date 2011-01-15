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
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/NumericalLearning/NumericalSupervisedInference.h>
# include <lbcpp/DecisionTree/DecisionTree.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

/*
** LearningMachineFamily
*/
class LearningMachineFamily : public Object
{
public:
  virtual InferencePtr createBinaryClassifier(PerceptionPtr perception, const Variable& arguments) const = 0;

  virtual InferencePtr createMultiClassClassifier(PerceptionPtr perception, EnumerationPtr labels, const Variable& arguments) const
  {
    // default: one-against-all
    return oneAgainstAllClassificationInference(T("MultiClass"), labels, createBinaryClassifier(perception, arguments));
  }

  virtual InferencePtr createMultiLabelClassifier(PerceptionPtr perception, EnumerationPtr labels, const Variable& arguments) const
  {
    // default: one-against-all multi-label
    return oneAgainstAllMultiLabelClassificationInference(T("MultiLabelMultiClass"), labels, createBinaryClassifier(perception, arguments));
  }
};

typedef ReferenceCountedObjectPtr<LearningMachineFamily> LearningMachineFamilyPtr;

class LinearLearningMachineFamily : public LearningMachineFamily
{
public:
  InferenceOnlineLearnerPtr createOnlineLearner(const Variable& arguments) const
  {
    InferenceOnlineLearnerPtr lastLearner;
    InferenceOnlineLearnerPtr res = lastLearner = gradientDescentOnlineLearner(perStep, constantIterationFunction(0.1));
    //lastLearner = lastLearner->setNextLearner(computeEvaluatorOnlineLearner(binaryClassificationConfusionEvaluator(T("binary"))));

    StoppingCriterionPtr criterion = maxIterationsStoppingCriterion(1000);//logicalOr(, maxIterationsWithoutImprovementStoppingCriterion(10));
    lastLearner = lastLearner->setNextLearner(stoppingCriterionOnlineLearner(criterion));
    return res;
  }

  virtual InferencePtr createBinaryClassifier(PerceptionPtr perception, const Variable& arguments) const
  {
    NumericalSupervisedInferencePtr inference = binaryLinearSVMInference(T("Classifier"), perception);
    inference->setStochasticLearner(createOnlineLearner(arguments), false);
    return inference;
  }

  virtual InferencePtr createMultiClassClassifier(PerceptionPtr perception, EnumerationPtr labels, const Variable& arguments) const
  {
    NumericalSupervisedInferencePtr inference = multiClassLinearSVMInference(T("Classifier"), perception, labels);
    inference->setStochasticLearner(createOnlineLearner(arguments), false);
    return inference;
  }
};

class ExtraTreeLearningMachineFamily : public LinearLearningMachineFamily
{
public:
  virtual InferencePtr createBinaryClassifier(PerceptionPtr perception, const Variable& arguments) const
    {return binaryClassificationExtraTreeInference(T("Classifier"), perception, 100, 10, 0);}

  virtual InferencePtr createMultiClassClassifier(PerceptionPtr perception, EnumerationPtr labels, const Variable& arguments) const
    {return classificationExtraTreeInference(T("Classifier"), perception, labels, 100, 10, 0);}
};

/*
** LearningProblem
*/
class LearningProblem : public Object
{
public:
  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file) = 0;
  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, const Variable& arguments) = 0;
  virtual EvaluatorPtr createEvaluator(ExecutionContext& context) = 0;
};

typedef ReferenceCountedObjectPtr<LearningProblem> LearningProblemPtr;

class MultiClassClassificationProblem : public LearningProblem
{
public:
  MultiClassClassificationProblem() : inputClass(new UnnamedDynamicClass(T("FeatureVector"))), outputLabels(new Enumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return classificationDataTextParser(context, file, inputClass.get(), outputLabels);}

  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, const Variable& arguments)
  {
    PerceptionPtr perception = addUnitFeatures(inputClass.get());
    return learningMachineFamily->createMultiClassClassifier(perception, outputLabels, arguments);
  }

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return classificationAccuracyEvaluator();}

protected:
  UnnamedDynamicClassPtr inputClass;
  EnumerationPtr outputLabels;
};

class MultiLabelClassificationProblem : public LearningProblem
{
public:
  MultiLabelClassificationProblem() : inputClass(new UnnamedDynamicClass(T("FeatureVector"))), outputLabels(new Enumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return multiLabelClassificationDataTextParser(context, file, inputClass.get(), outputLabels);}

  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, const Variable& arguments)
  {
    PerceptionPtr perception = identityPerception(inputClass.get());
    return learningMachineFamily->createMultiLabelClassifier(perception, outputLabels, arguments);
  }

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return multiLabelClassificationEvaluator();}

protected:
  UnnamedDynamicClassPtr inputClass;
  EnumerationPtr outputLabels;
};

class TrainTestLearningMachine : public WorkUnit
{
public:
  virtual bool run(ExecutionContext& context)
  {
    // check parameters
    if (!learningProblem)
    {
      context.errorCallback(T("Undefined learning problem"));
      return false;
    }
    if (!learningMachineFamily)
    {
      context.errorCallback(T("Undefined learning machine family"));
      return false;
    }

    // load training data
    ContainerPtr trainingData = loadData(context, learningProblem, trainingFile, T("training data"));
    if (!trainingData)
      return false;

    // load testing data
    ContainerPtr testingData = loadData(context, learningProblem, testingFile, T("testing data"));
    if (!testingData)
      return false;

    // create learning machine
    InferencePtr inference = learningProblem->createInference(context, learningMachineFamily, methodToUse);
    if (!inference)
      return false;

    // train
    if (!inference->train(context, trainingData, ContainerPtr(), T("Training")))
      return false;

    // evaluate on training data
    if (!inference->evaluate(context, trainingData, learningProblem->createEvaluator(context), T("Evaluate on training data")))
      return false;

    // evaluate on testing data
    if (!inference->evaluate(context, testingData, learningProblem->createEvaluator(context), T("Evaluate on testing data")))
      return false;

    return true;
  }

protected:
  friend class TrainTestLearningMachineClass;

  LearningProblemPtr learningProblem;
  LearningMachineFamilyPtr learningMachineFamily;
  String methodToUse;
  File trainingFile;
  File testingFile;
  size_t maxExamples;

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
    ContainerPtr res = stream->load(maxExamples);
    if (!res)
      return ContainerPtr();
    if (res->getNumElements() == 0)
      context.warningCallback(T("No ") + dataName);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_
