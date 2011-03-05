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
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

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
  RegressionProblem() : features(new DynamicClass(T("Features"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return regressionARFFDataParser(context, file, features);}

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return regressionEvaluator();}

protected:
  DynamicClassPtr features;
};

class BinaryClassificationProblem : public LearningProblem
{
public:
  BinaryClassificationProblem() : features(new DefaultEnumeration(T("Features"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return binaryClassificationLibSVMDataParser(context, file, features);}

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return binaryClassificationEvaluator();}

protected:
  DefaultEnumerationPtr features;
};

class MultiClassClassificationProblem : public LearningProblem
{
public:
  MultiClassClassificationProblem()
    : features(new DynamicClass(T("Features"))),
      labels(new DefaultEnumeration(T("Labels"))) {}

  virtual StreamPtr createDataParser(ExecutionContext& context, const File& file)
    {return classificationARFFDataParser(context, file, features, labels);}

  virtual EvaluatorPtr createEvaluator(ExecutionContext& context)
    {return classificationEvaluator();}

protected:
  DynamicClassPtr features;
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
    {return multiLabelClassificationLibSVMDataParser(context, file, features, labels);}

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
