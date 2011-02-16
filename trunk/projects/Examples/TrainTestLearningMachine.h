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

/*
** LearningProblem
*/
class LearningProblem : public Object
{
public:
  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file) = 0;
  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, size_t numStacks, const Variable& arguments) = 0;
  virtual EvaluatorPtr createEvaluator(ExecutionContext& context) = 0;
};

typedef ReferenceCountedObjectPtr<LearningProblem> LearningProblemPtr;

class MultiClassClassificationProblem : public LearningProblem
{
public:
  MultiClassClassificationProblem() : inputClass(new DynamicClass(T("FeatureVector"))), outputLabels(new DefaultEnumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return classificationDataTextParser(context, file, inputClass.get(), outputLabels);}

  virtual InferencePtr createInference(ExecutionContext& context, LearningMachineFamilyPtr learningMachineFamily, size_t numStacks, const Variable& arguments)
  {
    PerceptionPtr perception = addUnitFeatures(inputClass);
    return learningMachineFamily->createMultiClassClassifier(perception, outputLabels, arguments);
  }

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return classificationAccuracyEvaluator();}

protected:
  DynamicClassPtr inputClass;
  EnumerationPtr outputLabels;
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

class MultiLabelClassificationProblem : public LearningProblem
{
public:
  MultiLabelClassificationProblem()
    : inputClass(new DynamicClass(T("FeatureVector"))),
      outputLabels(new DefaultEnumeration(T("Labels"))),
      outputClass(enumBasedDoubleVectorClass(outputLabels, probabilityType))
  {
  }

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return multiLabelClassificationDataTextParser(context, file, inputClass.get(), outputLabels);}

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
  }

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return multiLabelClassificationEvaluator();}

protected:
  DynamicClassPtr inputClass;
  EnumerationPtr outputLabels;
  ClassPtr outputClass; 
};

class TrainTestLearningMachine : public WorkUnit
{
public:
  TrainTestLearningMachine() : numStacks(1), maxExamples(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    LearningProblemPtr learningProblem = createLearningProblem(context);
    LearningMachineFamilyPtr learningMachineFamily = createLearningMachineFamily(context);
    if (!learningProblem || !learningMachineFamily)
      return false;

    context.enterScope(T("Loading Data"));
    context.enterScope(T("Training Data"));
    ContainerPtr trainingData = loadData(context, learningProblem, trainingFile);
    context.leaveScope(trainingData != ContainerPtr());
    context.enterScope(T("Testing Data"));
    ContainerPtr testingData = loadData(context, learningProblem, testingFile);
    context.leaveScope(testingData != ContainerPtr());
    bool loadingOk = trainingData && testingData;
    context.leaveScope(loadingOk);
    if (!loadingOk)
      return false;
  
    // create learning machine
    InferencePtr inference = learningProblem->createInference(context, learningMachineFamily, numStacks, methodToUse);
    if (!inference)
      return false;

    // train
    if (!inference->train(context, trainingData, ContainerPtr(), T("Training")))
      return false;

    // tmp ! inference 
    context.resultCallback(T("inference"), inference);

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

  size_t learningProblem;
  size_t learningMachineFamily;
  size_t numStacks;
  String methodToUse;
  File trainingFile;
  File testingFile;
  size_t maxExamples;

  ContainerPtr loadData(ExecutionContext& context, LearningProblemPtr learningProblem, const File& file) const
  {
    if (!file.exists())
    {
      context.errorCallback(T("Missing file"));
      return ContainerPtr();
    }
    StreamPtr stream = learningProblem->createDataParser(context, file);
    if (!stream)
      return ContainerPtr();
    ContainerPtr res = stream->load(maxExamples);
    if (!res)
      return ContainerPtr();
    if (res->getNumElements() == 0)
      context.warningCallback(T("No examples"));
    context.resultCallback(T("Num Examples"), res->getNumElements());
    return res;
  }

  LearningProblemPtr createLearningProblem(ExecutionContext& context) const
  {
    static const juce::tchar* classNames[] = {T("MultiClassClassificationProblem"), T("MultiLabelClassificationProblem")};
    return createObjectFromEnum(context, learningProblem, classNames).staticCast<LearningProblem>();
  }

  LearningMachineFamilyPtr createLearningMachineFamily(ExecutionContext& context) const
  {
    static const juce::tchar* classNames[] = {T("LinearLearningMachineFamily"), T("ExtraTreeLearningMachineFamily")};
    return createObjectFromEnum(context, learningMachineFamily, classNames).staticCast<LearningMachineFamily>();
  }

  static ObjectPtr createObjectFromEnum(ExecutionContext& context, size_t enumValue, const juce::tchar* classNames[])
  {
    TypePtr type = typeManager().getType(context, classNames[enumValue]);
    return type ? Object::create(type) : ObjectPtr();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_TRAIN_TEST_LEARNING_MACHINE_H_
