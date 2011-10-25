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

class SimpleClassificationObjective : public SimpleUnaryFunction
{
public:
  SimpleClassificationObjective(ContainerPtr supervisions)
    : SimpleUnaryFunction(luapeGraphClass, doubleType), supervisions(supervisions) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const LuapeGraphPtr& graph = input.getObjectAndCast<LuapeGraph>();
    LuapeYieldNodePtr yieldNode = graph->getLastNode().dynamicCast<LuapeYieldNode>();
    if (!yieldNode)
      return DBL_MAX; // non-terminal state

    LuapeNodePtr valueNode = graph->getNode(yieldNode->getArgument());
    LuapeNodeCachePtr cache = valueNode->getCache();

    size_t numExamples = cache->getNumExamples();
    jassert(numExamples == supervisions->getNumElements());

    size_t numErrors = 0;
    for (size_t i = 0; i < numExamples; ++i)
      if (cache->getExample(i) != supervisions->getElement(i))
        ++numErrors;
    return numErrors / (double)numExamples;    
  }

protected:
  ContainerPtr supervisions;
};

class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxSteps(3) {}

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
    LuapeGraphPtr graph = problem->createInitialGraph(context);
    ContainerPtr supervisions;
    addExamplesToGraph(trainData, features, graph, supervisions);
    LuapeGraphCachePtr cache = new LuapeGraphCache();

    FunctionPtr objective = new SimpleClassificationObjective(supervisions);

    LuapeGraphBuilderStatePtr state = new LuapeGraphBuilderState(problem, graph, cache, maxSteps);
    double bestScore = DBL_MAX;
    exhaustiveSearch(context, state, objective, bestScore);
    return true;
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

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxSteps;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DefaultEnumerationPtr features) const
  {
    context.enterScope(T("Loading ") + file.getFileName());
    ContainerPtr res = binaryClassificationLibSVMFastParser(context, file, features)->load();
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  void exhaustiveSearch(ExecutionContext& context, LuapeGraphBuilderStatePtr state, const FunctionPtr& objective, double& bestScore)
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
        exhaustiveSearch(context, state, objective, bestScore);
      else
      {
        double score = objective->compute(context, state->getGraph()).toDouble();
        if (score != DBL_MAX)
        {
          //context.informationCallback(state->toShortString() + T(" => ") + String(score));
          if (score < bestScore)
          {
            bestScore = score;
            context.informationCallback(T("New best: ") + state->toShortString() + T(" [") + String(score) + T("]"));
          }
        }
      }
      state->undoTransition(context, action);
      //context.leaveScope(true);
    }
  }

};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SAND_BOX_H_
