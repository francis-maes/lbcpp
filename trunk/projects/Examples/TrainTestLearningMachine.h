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
# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include <lbcpp/DecisionTree/DecisionTree.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Function/StoppingCriterion.h>

# include <lbcpp/Inference/SequentialInference.h>

namespace lbcpp
{

#if 0
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

    StoppingCriterionPtr criterion = maxIterationsStoppingCriterion(100);//logicalOr(, maxIterationsWithoutImprovementStoppingCriterion(10));
    criterion = logicalOrStoppingCriterion(criterion, isAboveValueStoppingCriterion(0.0));
    lastLearner = lastLearner->setNextLearner(oldStoppingCriterionOnlineLearner(criterion));
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

class StackedSequentialInference : public VectorSequentialInference
{
public:
  StackedSequentialInference(InferencePtr firstStack, InferencePtr nextStacksModel, size_t numStacks)
    : VectorSequentialInference(T("Stacked"))
  {
    jassert(numStacks >= 1);
    subInferences.resize(numStacks);
    subInferences[0] = firstStack;
    for (size_t i = 1; i < numStacks; ++i)
      subInferences[i] = nextStacksModel->cloneAndCast<Inference>();
  }
  StackedSequentialInference() {}

  virtual TypePtr getInputType() const
    {return subInferences[0]->getInputType();}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return subInferences.back()->getOutputType(pairClass(getInputType(), subInferences[0]->getOutputType(inputType)));}

  virtual void prepareSubInference(ExecutionContext& context, SequentialInferenceStatePtr state, size_t index) const
    {state->setSubInference(subInferences[index], index == 0 ? state->getInput() : new Pair(state->getInput(), state->getSubOutput()), state->getSupervision());}
};
#endif // 0


/*
** LearningProblem
*/
class LearningProblem;
typedef ReferenceCountedObjectPtr<LearningProblem> LearningProblemPtr;

class LearningProblem : public Object
{
public:
  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file) = 0;
  //virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, size_t numStacks, const Variable& arguments) = 0;
  virtual EvaluatorPtr createEvaluator(ExecutionContext& context) = 0;

  virtual String toString() const
    {return getClass()->getShortName();}
};

class RegressionProblem : public LearningProblem
{
public:
  RegressionProblem() : features(new DefaultEnumeration(T("Features"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return regressionDataTextParser(context, file, features);}

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return regressionErrorEvaluator(T("regression"));}

protected:
  DefaultEnumerationPtr features;
};

class BinaryClassificationProblem : public LearningProblem
{
public:
  BinaryClassificationProblem() : features(new DefaultEnumeration(T("Features"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return binaryClassificationDataTextParser(context, file, features);}

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return binaryClassificationConfusionEvaluator(T("binary"));}

protected:
  DefaultEnumerationPtr features;
};

class MultiClassClassificationProblem : public LearningProblem
{
public:
  MultiClassClassificationProblem()
    : features(new DefaultEnumeration(T("Features"))),
      labels(new DefaultEnumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return classificationDataTextParser(context, file, features, labels);}
/*
  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, size_t numStacks, const Variable& arguments)
  {
    PerceptionPtr perception = addUnitFeatures(inputClass);
    return learningMachineFamily->createMultiClassClassifier(perception, outputLabels, arguments);
  }*/

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return classificationAccuracyEvaluator();}

protected:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
};

class MultiLabelClassificationProblem : public LearningProblem
{
public:
  MultiLabelClassificationProblem()
    : features(new DefaultEnumeration(T("Features"))),
      labels(new DefaultEnumeration(T("Labels"))),
      outputClass(sparseDoubleVectorClass(labels, probabilityType))
  {
  }

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return multiLabelClassificationDataTextParser(context, file, features, labels);}
/*
  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, size_t numStacks, const Variable& arguments)
  {
    PerceptionPtr firstStackPerception = identityPerception(inputClass);
    InferencePtr firstStack = learningMachineFamily->createMultiLabelClassifier(firstStackPerception, outputLabels, arguments);
    if (numStacks <= 1)
      return firstStack;
    else
    {
      PerceptionPtr nextStacksPerception = concatenatePairPerception(firstStackPerception, conjunctionFeatures(identityPerception(outputClass), identityPerception(outputClass)));
      //PerceptionPtr nextStacksPerception = conjunctionFeatures(firstStackPerception, identityPerception(outputClass), false);
      InferencePtr nextStacksModel = learningMachineFamily->createMultiLabelClassifier(identityPerception(nextStacksPerception->getOutputType()), outputLabels, arguments);
      nextStacksModel = preProcessInference(nextStacksModel, nextStacksPerception);
      return new StackedSequentialInference(firstStack, nextStacksModel, numStacks);
    }
  }*/

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return multiLabelClassificationEvaluator();}

protected:
  DefaultEnumerationPtr features;
  DefaultEnumerationPtr labels;
  ClassPtr outputClass; 
};

class TrainTestLearningMachine : public WorkUnit
{
public:
  TrainTestLearningMachine() : maxExamples(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!learningProblem || !learningMachine)
      return false;

    // load data
    context.enterScope(T("Loading Data"));
    ContainerPtr trainingData = loadData(context, learningProblem, trainingFile, T("Training Data"));
    ContainerPtr testingData = loadData(context, learningProblem, testingFile, T("Testing Data"));
    bool loadingOk = trainingData && testingData;
    context.leaveScope(loadingOk);
    if (!loadingOk)
      return false;

    // initialize learning machine
    TypePtr examplesType = trainingData->getElementsType();
    jassert(examplesType->getNumTemplateArguments() == 2);
    if (!learningMachine->initialize(context, examplesType->getTemplateArgument(0), examplesType->getTemplateArgument(1)))
      return false;

    // train
    if (!learningMachine->train(context, trainingData, ContainerPtr(), T("Training"), true))
      return false;

    // evaluate on training data
    if (!learningMachine->evaluate(context, trainingData, learningProblem->createEvaluator(context), T("Evaluate on training data")))
      return false;
    
    // evaluate on testing data
    if (!learningMachine->evaluate(context, testingData, learningProblem->createEvaluator(context), T("Evaluate on testing data")))
      return false;

    return true;
  }

protected:
  friend class TrainTestLearningMachineClass;

  LearningProblemPtr learningProblem;
  FunctionPtr learningMachine;

  //size_t numStacks;
  //String methodToUse;
  File trainingFile;
  File testingFile;
  size_t maxExamples;

  ContainerPtr loadData(ExecutionContext& context, LearningProblemPtr learningProblem, const File& file, const String& scopeName) const
  {
    context.enterScope(scopeName);

    ContainerPtr res;
    if (!file.exists())
      context.errorCallback(T("Missing file"));
    else
    {
      StreamPtr stream = learningProblem->createDataParser(context, file);
      if (stream)
      {
        res = stream->load(maxExamples);
        if (res && res->getNumElements() == 0)
          context.warningCallback(T("No examples"));
      }
    }

    context.leaveScope(String((int)(res ? res->getNumElements() : 0)) + T(" examples"));
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_
