/*-----------------------------------------.---------------------------------.
| Filename: LuapeSandBox.h                 | Luape Sand Box                  |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_SAND_BOX_H_
# define LBCPP_LUAPE_SAND_BOX_H_

# include <lbcpp/Data/Stream.h>
# include "LuapeProblem.h"
# include "LuapeBatchLearner.h"

namespace lbcpp
{

class ProductFunction : public SimpleBinaryFunction
{
public:
  ProductFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "prod") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() * inputs[1].toDouble();}
};

class LogFunction : public SimpleUnaryFunction
{
public:
  LogFunction() : SimpleUnaryFunction(doubleType, doubleType, "log") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return log(input.toDouble());}
};

class StumpFunction : public SimpleUnaryFunction
{
public:
  StumpFunction(double threshold = 0.0) 
    : SimpleUnaryFunction(doubleType, booleanType, "stump"), threshold(threshold) {}

  virtual String toShortString() const
    {return T("Stump(") + String(threshold) + T(")");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.toDouble() >= threshold;}

protected:
  friend class StumpFunctionClass;

  double threshold;
};

class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxExamples(0), maxSteps(3), maxIterations(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr features = new DefaultEnumeration("features");
    ContainerPtr trainData = loadData(context, trainFile, features);
    ContainerPtr testData = loadData(context, testFile, features);
    if (!trainData || !testData)
      return false;

    context.informationCallback(
      String((int)trainData->getNumElements()) + T(" training examples, ") +
      String((int)testData->getNumElements()) + T(" testing examples, ") + 
      String((int)features->getNumElements()) + T(" features"));

    LuapeProblemPtr problem = createProblem(features);

    LuapeFunctionPtr classifier = new LuapeBinaryClassifier();
    classifier->setBatchLearner(new AdaBoostLuapeLearner(problem, maxSteps, maxIterations));
    classifier->setEvaluator(binaryClassificationEvaluator());

    classifier->train(context, trainData, testData, T("Training"), true);
    classifier->evaluate(context, trainData, EvaluatorPtr(), T("Evaluating on training data"));
    classifier->evaluate(context, testData, EvaluatorPtr(), T("Evaluating on testing data"));
/*
    LuapeGraphPtr initialGraph = problem->createInitialGraph(context);
//    ContainerPtr supervisions;
//    addExamplesToGraph(trainData, features, initialGraph, supervisions);

    LuapeBatchLearnerPtr learner = new AdaBoostLuapeLearner(problem, maxSteps, 10);
    DenseDoubleVectorPtr alphas;
    LuapeGraphPtr graph = learner->learnGraph(context, initialGraph, trainData, alphas);
    context.resultCallback("initialGraph", initialGraph);
    context.resultCallback("graph", graph);
    context.resultCallback("alphas", alphas);
    evaluateGraph(context, graph, trainData, features, "training data");
    evaluateGraph(context, graph, testData, features, "testing data");*/
    return true;
  }

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxExamples;
  size_t maxSteps;
  size_t maxIterations;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DefaultEnumerationPtr features) const
  {
    context.enterScope(T("Loading ") + file.getFileName());
    ContainerPtr res = binaryClassificationLibSVMFastParser(context, file, features)->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  void addExamplesToGraph(ContainerPtr trainData, EnumerationPtr features, LuapeGraphPtr graph, ContainerPtr& supervisions) const
  {
    size_t n = trainData->getNumElements();
    size_t nf = features->getNumElements();

    supervisions = vector(trainData->getElementsType()->getTemplateArgument(1), n);

    graph->reserveExamples(n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = trainData->getElement(i).getObjectAndCast<Pair>();
      DoubleVectorPtr doubleVector = example->getFirst().getObjectAndCast<DoubleVector>();
      std::vector<Variable> graphExample(nf);
      for (size_t j = 0; j < nf; ++j)
        graphExample[j] = doubleVector->getElement(j);
      graph->addExample(graphExample);
      supervisions->setElement(i, example->getSecond());
    }
  }

  LuapeProblemPtr createProblem(const EnumerationPtr& features)
  {
    LuapeProblemPtr res = new LuapeProblem();
    size_t n = features->getNumElements();
    for (size_t i = 0; i < n; ++i)
      res->addInput(doubleType, features->getElementName(i));

    res->addFunction(new LogFunction());
    //res->addFunction(new ProductFunction());
    for (double k = -1.0; k <= 1.0; k += 0.2)
      res->addFunction(new StumpFunction(k));
    return res;
  }

  void evaluateGraph(ExecutionContext& context, const LuapeGraphPtr& graph, const ContainerPtr& data, EnumerationPtr features, const String& dataName)
  {
    context.enterScope(T("Evaluating on ") + dataName);

    size_t numExamples = data->getNumElements();
    size_t numErrors = 0;
    for (size_t i = 0; i < numExamples; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      DoubleVectorPtr doubleVector = example->getFirst().getObjectAndCast<DoubleVector>();

      std::vector<Variable> state(graph->getNumNodes());
      for (size_t j = 0; j < features->getNumElements(); ++j)
        state[j] = doubleVector->getElement(j);

      graph->compute(context, state, features->getNumElements());
      Variable prediction = state.back();
      if (prediction != example->getSecond())
        ++numErrors;
      
    }

    double accuracy = (numExamples - numErrors) / (double)numExamples;
    context.informationCallback(T("Accuracy: ") + String((int)(numExamples - numErrors)) + T(" / ") + String((int)numExamples) + T(" (") +
              String(accuracy * 100.0, 2) + T(" % accuracy)"));
    context.leaveScope(accuracy);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SAND_BOX_H_
