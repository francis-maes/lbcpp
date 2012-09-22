
# include <lbcpp/Core/Function.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

/***** Function *****/
class AgAddFunction : public SimpleBinaryFunction
{
public:
  AgAddFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, T("Add")) {}

  virtual String toString() const
    {return T("+");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgSubFunction : public SimpleBinaryFunction
{
public:
  AgSubFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, T("Sub")) {}

  virtual String toString() const
    {return T("-");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgMultiplyFunction : public SimpleBinaryFunction
{
public:
  AgMultiplyFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, T("Multiply")) {}

  virtual String toString() const
    {return T("*");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 1.0;
    for (size_t i = 0; i < n; ++i)
      res *= inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgDivideFunction : public SimpleBinaryFunction
{
public:
  AgDivideFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, T("Divide")) {}

  virtual String toString() const
    {return T("/");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 1.0;
    for (size_t i = 0; i < n; ++i)
    {
      double value = inputs[i].getDouble();
      res /= (value > 10e-6) ? value : 10e-6;
    }
    return Variable(res, doubleType);
  }
};

/***** Tree *****/
class GeneticTree : public Object
{
public:
  size_t createLeaf(size_t parentNode, size_t variableIndex)
  {
    size_t res = nodes.size();
    nodes.push_back(Node(variableIndex));
    appendChild(parentNode, res);
    return res;
  }

  size_t createInternalNode(size_t parentNode, FunctionPtr function)
  {
    size_t res = nodes.size();
    nodes.push_back(Node(function));
    appendChild(parentNode, res);
    return res;
  }
  
  void setVariableIndex(size_t node, size_t variableIndex)
    {jassert(isLeaf(node)); nodes[node].value.variableIndex = variableIndex;}
  
  void setFunction(size_t node, FunctionPtr function)
    {jassert(isInternalNode(node)); nodes[node].value.function = function.get();}
  
  size_t getChild(size_t node, size_t index) const
  {
    jassert(node < nodes.size() && index < nodes[node].children.size());
    return nodes[node].children[index];
  }
  
  bool isLeaf(size_t node) const
    {jassert(node < nodes.size()); return nodes[node].children.size() == 0;}
  
  bool isInternalNode(size_t node) const
    {jassert(node < nodes.size()); return nodes[node].children.size() != 0;}
  
  size_t getVariableIndex(size_t node) const
    {jassert(isLeaf(node)); return nodes[node].value.variableIndex;}
  
  FunctionPtr getFunction(size_t node) const
    {jassert(isInternalNode(node)); return nodes[node].value.function;}
  
  size_t getNumNodes() const
    {return nodes.size();}
  
  size_t getNumChildren(size_t node) const
    {jassert(node < nodes.size()); return nodes[node].children.size();}
  
  virtual String toString(size_t node = 0)
  {
    if (isLeaf(node))
      return String((int)nodes[node].value.variableIndex);
    FunctionPtr function = nodes[node].value.function;
    String res = T("(") + function->toString();
    for (size_t i = 0; i < nodes[node].children.size(); ++i)
      res += T(" ") + toString(getChild(node, i));
    return res + T(")");
  }

protected:
  class Node
  {
  public:
    Node(size_t variableIndex)
    {
      value.variableIndex = variableIndex;
    }
    
    Node(FunctionPtr function)
    {
      value.function = function.get();
    }

    union
    {
      Function* function;
      size_t variableIndex;
    } value;
    std::vector<size_t> children;
  };
  std::vector<Node> nodes;

  void appendChild(size_t parentNode, size_t childNode)
  {
    if (childNode == 0)
      return;
    jassert(parentNode < nodes.size() && childNode < nodes.size());
    nodes[parentNode].children.push_back(childNode);
  }
};

typedef ReferenceCountedObjectPtr<GeneticTree> GeneticTreePtr;

void createRandomGeneticTree(RandomGeneratorPtr& random, size_t numInputVariables, size_t maxDepth, std::vector<FunctionPtr>& functions, GeneticTreePtr& tree, size_t node)
{
  double probOfLeaf = (double)numInputVariables / (functions.size() * maxDepth + numInputVariables);
  if (random->sampleDouble() < probOfLeaf)
  {
    tree->createLeaf(node, random->sampleSize(numInputVariables));
    return;
  }

  size_t selectedFunction = random->sampleSize(functions.size());
  size_t newNode = tree->createInternalNode(node, functions[selectedFunction]);
  for (size_t i = 0; i < functions[selectedFunction]->getNumInputs(); ++i)
    createRandomGeneticTree(random, numInputVariables, maxDepth - 1, functions, tree, newNode);
}

Variable makePredictionTree(ExecutionContext& context, const GeneticTreePtr& tree, size_t node, const std::vector<double>& inputs)
{
  if (tree->isLeaf(node))
    return Variable(inputs[tree->getVariableIndex(node)], doubleType);
  FunctionPtr function = tree->getFunction(node);
  size_t n = function->getNumInputs();
  std::vector<Variable> results(n);
  for (size_t i = 0; i < n; ++i)
    results[i] = makePredictionTree(context, tree, tree->getChild(node, i), inputs);
  return function->compute(context, results);
}

double evaluateTree(ExecutionContext& context, const GeneticTreePtr& tree, const std::vector<std::vector<double> >& inputData, const std::vector<double>& outputData)
{
  jassertfalse; // broken
/*  EvaluatorPtr evaluator = EvaluatorPtr();
  for (size_t i = 0; i < inputData.size(); ++i)
    evaluator->addPrediction(context, makePredictionTree(context, tree, 0, inputData[i]), Variable(outputData[i], doubleType));
  return evaluator->getDefaultScore();*/
  return 0.0;
}

void inducePointMutation(RandomGeneratorPtr& random, size_t numInputVariables, const std::vector<FunctionPtr>& functions, const GeneticTreePtr& tree, size_t node, double probOfMutation)
{
  if (random->sampleDouble() < probOfMutation)
  {
    if (tree->isLeaf(node))
      tree->setVariableIndex(node, random->sampleSize(numInputVariables));
    else
      tree->setFunction(node, functions[random->sampleSize(functions.size())]);
  }

  if (tree->isLeaf(node))
    return;

  for (size_t i = 0; i < tree->getNumChildren(node); ++i)
    inducePointMutation(random, numInputVariables, functions, tree, tree->getChild(node, i), probOfMutation);
}

void makeCrossOver(size_t breakNode, const GeneticTreePtr& from, size_t fromNode, const GeneticTreePtr& alter, size_t alterNode, GeneticTreePtr& to, size_t toNode)
{
  if (fromNode == breakNode)
  {
    makeCrossOver(alter->getNumNodes(), alter, alterNode, from, fromNode, to, toNode);
    return;
  }
  
  if (from->isLeaf(fromNode))
  {
    to->createLeaf(toNode, from->getVariableIndex(fromNode));
    return;
  }
  
  size_t parentNode = to->createInternalNode(toNode, from->getFunction(fromNode));
  size_t n = from->getNumChildren(fromNode);
  for (size_t i = 0; i < n; ++i)
    makeCrossOver(breakNode, from, from->getChild(fromNode, i), alter, alterNode, to, parentNode);
}

void makeCrossOver(RandomGeneratorPtr& random, GeneticTreePtr& a, GeneticTreePtr& b)
{
  size_t indexA = random->sampleSize(a->getNumNodes());
  size_t indexB = random->sampleSize(b->getNumNodes());
  
  GeneticTreePtr newA = new GeneticTree();
  makeCrossOver(indexA, a, 0, b, indexB, newA, 0);
  GeneticTreePtr newB = new GeneticTree();
  makeCrossOver(indexB, b, 0, a, indexA, newB, 0);
  
  a = newA;
  b = newB;
}

class GeneticProgrammingExample : public WorkUnit
{
public:
  GeneticProgrammingExample() : random(new RandomGenerator(1)), 
                                numInputVariables(10), numData(20), numTrees(20), numGenerations(5),
                                probOfCrossOver(0.9), probOfReproduction(0.08), probOfMutation(0.02) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    /* Functions set */
    std::vector<FunctionPtr> functions;
    functions.push_back(new AgAddFunction());
    functions.push_back(new AgMultiplyFunction());
    functions.push_back(new AgSubFunction());
    functions.push_back(new AgDivideFunction());

    for (size_t i = 0; i < functions.size(); ++i)
      functions[i]->initialize(context, doubleType, doubleType);
    
    /* Data */
    generateData();

    /* Initial trees */
    context.enterScope(T("Initialization"));
    ProgressionStatePtr progress = new ProgressionState(0, numTrees, T("Tree"));
    trees.resize(numTrees);
    for (size_t i = 0; i < numTrees; ++i)
    {
      trees[i] = new GeneticTree();
      createRandomGeneticTree(random, numInputVariables, 5, functions, trees[i], 0);
      progress->setValue(i + 1);
      context.progressCallback(progress);
    }

    std::vector<double> results;
    evaluateTrees(context, results);
    context.leaveScope(Variable());
    
    /* Generation */
    enum {crossOver, pointMutation, reproduction, bestTree};
    progress = new ProgressionState(0, numGenerations, T("Generation"));
    for (size_t i = 0; i < numGenerations; ++i)
    {
      context.enterScope(T("Generation ") + String((int)i + 1));
      
      std::vector<int> operation(numTrees);
      for (size_t j = 0; j < numTrees; ++j)
      {
        double p = random->sampleDouble();
        if (p < probOfCrossOver)
          operation[j] = crossOver;
        else
          operation[j] = reproduction;
      }
      
      /* Force the best tree to be reproduced */
      double bestScore = -DBL_MAX;
      size_t bestIndex = numTrees;
      for (size_t j = 0; j < numTrees; ++j)
        if (results[j] > bestScore)
        {
          bestScore = results[j];
          bestIndex = j;
        }
      jassert(bestIndex < numTrees);
      operation[bestIndex] = bestTree;

      /* CrossOver */
      std::vector<size_t> indexToCrossOver;
      for (size_t j = 0; j < numTrees; ++j)
        if (operation[j] == crossOver)
          indexToCrossOver.push_back(j);
      std::vector<size_t> sampledIndexToCrossOver(indexToCrossOver.size());
      random->sampleOrder(0, indexToCrossOver.size(), sampledIndexToCrossOver);
      for (size_t j = 0; j < indexToCrossOver.size(); ++j)
        sampledIndexToCrossOver[j] = indexToCrossOver[sampledIndexToCrossOver[j]];
      
      for (size_t j = 0; j < sampledIndexToCrossOver.size() / 2; j += 2)
        makeCrossOver(random, trees[sampledIndexToCrossOver[j]], trees[sampledIndexToCrossOver[j + 1]]);

      /* Induce point mutation */
      for (size_t j = 0; j < numTrees; ++j)
        if (operation[j] != bestTree)
          inducePointMutation(random, numInputVariables, functions, trees[j], 0, probOfMutation);

      /* Evaluate */
      evaluateTrees(context, results);

      context.leaveScope(Variable());
      progress->setValue(i + 1);
      context.progressCallback(progress);
    }

    return Variable();
  }
  
protected:
  friend class GeneticProgrammingExampleClass;
  
  RandomGeneratorPtr random;
  size_t numInputVariables;
  size_t numData;
  size_t numTrees;
  size_t numGenerations;
  double probOfCrossOver;
  double probOfReproduction;
  double probOfMutation;
  
  std::vector<std::vector<double> > inputData;
  std::vector<double> outputData;
  std::vector<GeneticTreePtr> trees;
  
  void generateData()
  {
    jassert(numData > 5);
    inputData.resize(numData);
    outputData.resize(numData);
    for (size_t i = 0; i < numData; ++i)
    {
      std::vector<double> data(numInputVariables);
      for (size_t j = 0; j < numInputVariables; ++j)
        data[j] = random->sampleDouble(numInputVariables);
      data[5] = i;
      inputData[i] = data;
      outputData[i] = i * i;
    }
  }
  
  void evaluateTrees(ExecutionContext& context, std::vector<double>& results) const
  {
    context.enterScope(T("Evaluation"));
    ProgressionStatePtr progress = new ProgressionState(0, numTrees, T("Tree"));
    results.resize(numTrees);
    for (size_t i = 0; i < trees.size(); ++i)
    {
      results[i] = evaluateTree(context, trees[i], inputData, outputData);
      context.enterScope(T("Tree ") + String((int)i));
      context.resultCallback(T("Iteration"), Variable((int)i, positiveIntegerType));
      context.resultCallback(T("Tree"), trees[i]->toString());
      context.resultCallback(T("Score"), Variable(results[i], doubleType));
      context.leaveScope(Variable());
      progress->setValue(i + 1);
      context.progressCallback(progress);
    }
    context.leaveScope(Variable());
  }
  
  void printMeanSizeOfTrees() const
  {
    ScalarVariableMeanPtr meanTree = new ScalarVariableMean();
    for (size_t i = 0; i < numTrees; ++i)
      meanTree->push(trees[i]->getNumNodes());
    std::cout << "Mean: " << meanTree->getMean() << std::endl;
  }
};

};
