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


/////////// LuapeLearner

class LuapeLearner : public Object
{
public:
  LuapeLearner(LuapeProblemPtr problem = LuapeProblemPtr())
    : problem(problem) {}

  virtual LuapeGraphPtr learnGraph(ExecutionContext& context, LuapeGraphPtr initialGraph, const ContainerPtr& examples, DenseDoubleVectorPtr& alphas) const = 0;

protected:
  LuapeProblemPtr problem;
};

typedef ReferenceCountedObjectPtr<LuapeLearner> LuapeLearnerPtr;

class BoostingLuapeLearner : public LuapeLearner
{
public:
  BoostingLuapeLearner(LuapeProblemPtr problem, size_t maxSteps, size_t maxIterations)
    : LuapeLearner(problem), maxSteps(maxSteps), maxIterations(maxIterations) {}
  BoostingLuapeLearner() : maxIterations(0) {}

  virtual DenseDoubleVectorPtr makeInitialWeights(const ContainerPtr& examples) const = 0;

  // the absolute value of this quantity should be maximized
  virtual double computeWeakObjective(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const = 0;
  virtual bool shouldStop(double weakObjectiveValue) const = 0;
  virtual double computeAlpha(double weakObjectiveValue) const = 0;
  virtual double updateWeight(const Variable& prediction, const Variable& supervision, double currentWeight, double alpha) const = 0;

  virtual LuapeGraphPtr learnGraph(ExecutionContext& context, LuapeGraphPtr initialGraph, const ContainerPtr& examples, DenseDoubleVectorPtr& alphas) const
  {
    DenseDoubleVectorPtr weights = makeInitialWeights(examples);
    LuapeGraphPtr graph = initialGraph->cloneAndCast<LuapeGraph>();
    VectorPtr supervisions;
    addExamplesToGraph(examples, graph, supervisions);
    alphas = new DenseDoubleVector(0, 0.0);

    bool stopped = false;
    for (size_t i = 0; i < maxIterations && !stopped; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i+1));
      context.resultCallback(T("iteration"), i + 1);

      // weak graph completion
      LuapeGraphPtr newGraph = learnWeakModel(context, graph, weights, supervisions);
      LuapeYieldNodePtr yieldNode = newGraph->getLastNode().dynamicCast<LuapeYieldNode>();
      jassert(yieldNode);
      double score = yieldNode->getCache()->getScore();
      LuapeNodeCachePtr cache = newGraph->getNode(yieldNode->getArgument())->getCache();

      // stop test
      if (shouldStop(score))
      {
        context.informationCallback(T("Stopping, score = ") + String(score));
        stopped = true;
      }
      else
      {
        graph = newGraph;

        // compute alpha
        double alpha = computeAlpha(score);
        alphas->appendValue(alpha);

        // update weights
        updateWeights(cache->getExamples(), supervisions, weights, alpha);

        context.resultCallback("score", score);
        context.resultCallback("alpha", alpha);
        context.resultCallback("weights", weights->cloneAndCast<DoubleVector>());
      }

      context.leaveScope();
      context.progressCallback(new ProgressionState(i+1, maxIterations, T("Iterations")));
    }
    return graph;
  }

protected:
  size_t maxSteps;
  size_t maxIterations;

  void addExamplesToGraph(const ContainerPtr& examples, LuapeGraphPtr graph, VectorPtr& supervisions) const
  {
    size_t n = examples->getNumElements();
    graph->reserveExamples(n);
    supervisions = vector(examples->getElementsType()->getTemplateArgument(1), n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = examples->getElement(i).getObjectAndCast<Pair>();
      graph->addExample(example->getFirst().getObjectAndCast<Container>());
      supervisions->setElement(i, example->getSecond());
    }
  }

  void updateWeights(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights, double alpha) const
  {
    size_t n = predictions->getNumElements();
    jassert(n == supervisions->getNumElements());
    jassert(n == weights->getNumElements());

    double* values = weights->getValuePointer(0);
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      values[i] = updateWeight(predictions->getElement(i), supervisions->getElement(i), values[i], alpha);
      sum += values[i];
    }
    weights->multiplyByScalar(1.0 / sum);
  }

  void exhaustiveSearch(ExecutionContext& context, LuapeGraphBuilderStatePtr state, const FunctionPtr& objective, double& bestScore, LuapeGraphPtr& bestGraph) const
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable action = actions->getElement(i);
      double reward;
      //context.enterScope(action.toShortString());
      state->performTransition(context, action, reward);

      if (!state->isFinalState())
        exhaustiveSearch(context, state, objective, bestScore, bestGraph);
      else
      {
        double score = objective->compute(context, state->getGraph()).toDouble();
        if (score != DBL_MAX)
        {
          //context.informationCallback(state->toShortString() + T(" => ") + String(score));
          if (score < bestScore)
          {
            bestScore = score;
            bestGraph = state->getGraph()->cloneAndCast<LuapeGraph>();
            context.informationCallback(T("New best: ") + state->toShortString() + T(" [") + String(score) + T("]"));
          }
        }
      }
      state->undoTransition(context, action);
      //context.leaveScope(true);
    }
  }


  struct Objective : public SimpleUnaryFunction
  {
    Objective(const BoostingLuapeLearner* pthis, ContainerPtr supervisions, DenseDoubleVectorPtr weights)
      : SimpleUnaryFunction(luapeGraphClass, doubleType), pthis(pthis), supervisions(supervisions), weights(weights) {}

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      const LuapeGraphPtr& graph = input.getObjectAndCast<LuapeGraph>();
      LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
      if (!yieldNode)
        return DBL_MAX; // non-terminal state

      LuapeNodePtr valueNode = graph->getNode(yieldNode->getArgument());
      LuapeNodeCachePtr cache = valueNode->getCache();
      double score = pthis->computeWeakObjective(cache->getExamples(), supervisions, weights);
      yieldNode->getCache()->setScore(score);
      return -fabs(score);
    }

  protected:
    const BoostingLuapeLearner* pthis;
    ContainerPtr supervisions;
    DenseDoubleVectorPtr weights;
  };

  LuapeGraphPtr learnWeakModel(ExecutionContext& context, LuapeGraphPtr graph, const DenseDoubleVectorPtr& weights, const ContainerPtr& supervisions) const
  {
    FunctionPtr objective = new Objective(this, supervisions, weights);
    LuapeGraphCachePtr cache = new LuapeGraphCache();
    LuapeGraphBuilderStatePtr state = new LuapeGraphBuilderState(problem, graph, cache, maxSteps);
    double bestScore = DBL_MAX;

    LuapeGraphPtr bestGraph;
    exhaustiveSearch(context, state, objective, bestScore, bestGraph);
    return bestGraph;
  }
};

class AdaBoostLuapeLearner : public BoostingLuapeLearner
{
public:
  AdaBoostLuapeLearner(LuapeProblemPtr problem, size_t maxSteps, size_t maxIterations)
    : BoostingLuapeLearner(problem, maxSteps, maxIterations) {}
  AdaBoostLuapeLearner() {}

  virtual DenseDoubleVectorPtr makeInitialWeights(const ContainerPtr& examples) const
    {size_t n = examples->getNumElements(); return new DenseDoubleVector(n, 1.0 / n);}

  virtual double computeWeakObjective(const ContainerPtr& predictions, const ContainerPtr& supervisions, const DenseDoubleVectorPtr& weights) const
  {
    size_t numExamples = predictions->getNumElements();
    jassert(numExamples == supervisions->getNumElements());
    jassert(numExamples == weights->getNumElements());

    double accuracy = 0.0;
    for (size_t i = 0; i < numExamples; ++i)
      if (predictions->getElement(i) == supervisions->getElement(i))
        accuracy += weights->getValue(i);
    return accuracy - 0.5;
  }

  virtual bool shouldStop(double weakObjectiveValue) const
    {return weakObjectiveValue == 0.0 || weakObjectiveValue == 0.5;}

  virtual double computeAlpha(double weakObjectiveValue) const
  {
    double accuracy = weakObjectiveValue + 0.5;
    if (accuracy == 0.0)
      return -1.0;
    else if (accuracy == 1.0)
      return 1.0;
    else
      return 0.5 * log((1.0 - accuracy) / accuracy);
  }

  virtual double updateWeight(const Variable& prediction, const Variable& supervision, double currentWeight, double alpha) const
    {return currentWeight * exp(alpha * (supervision.getBoolean() == prediction.getBoolean() ? 1.0 : -1.0));}
};

class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxExamples(0), maxSteps(3) {}

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
    LuapeGraphPtr initialGraph = problem->createInitialGraph(context);
//    ContainerPtr supervisions;
//    addExamplesToGraph(trainData, features, initialGraph, supervisions);

    LuapeLearnerPtr learner = new AdaBoostLuapeLearner(problem, maxSteps, 10);
    DenseDoubleVectorPtr alphas;
    LuapeGraphPtr graph = learner->learnGraph(context, initialGraph, trainData, alphas);
    context.resultCallback("initialGraph", initialGraph);
    context.resultCallback("graph", graph);
    context.resultCallback("alphas", alphas);
    evaluateGraph(context, graph, trainData, features, "training data");
    evaluateGraph(context, graph, testData, features, "testing data");
    return true;
  }

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxExamples;
  size_t maxSteps;

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
