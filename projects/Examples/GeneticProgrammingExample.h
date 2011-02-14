
# include <lbcpp/Function/Function.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Data/RandomVariable.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

/***** Function *****/
class AgAddFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual String getOuputPostFix() const
    {return T("Add");}
  
  virtual String toString() const
    {return T("+");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgSubFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual String getOuputPostFix() const
    {return T("Sub");}
  
  virtual String toString() const
    {return T("-");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 0.0;
    for (size_t i = 0; i < n; ++i)
      res += inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgMultiplyFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual String getOuputPostFix() const
    {return T("Multiply");}
  
  virtual String toString() const
    {return T("*");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t n = getNumInputs();
    double res = 1.0;
    for (size_t i = 0; i < n; ++i)
      res *= inputs[i].getDouble();
    return Variable(res, doubleType);
  }
};

class AgDivideFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual String getOuputPostFix() const
    {return T("Divide");}
  
  virtual String toString() const
    {return T("/");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
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
  Variable results[n];
  for (size_t i = 0; i < n; ++i)
    results[i] = makePredictionTree(context, tree, tree->getChild(node, i), inputs);
  return function->compute(context, results);
}

double evaluateTree(ExecutionContext& context, const GeneticTreePtr& tree, const std::vector<std::vector<double> >& inputData, const std::vector<double>& outputData)
{
  EvaluatorPtr evaluator = regressionErrorEvaluator(T("Eval"));
  for (size_t i = 0; i < inputData.size(); ++i)
    evaluator->addPrediction(context, makePredictionTree(context, tree, 0, inputData[i]), Variable(outputData[i], doubleType));
  return evaluator->getDefaultScore();
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
    {
      std::vector<VariableSignaturePtr> inputVariables;
      inputVariables.push_back(new VariableSignature(doubleType, T("a")));
      inputVariables.push_back(new VariableSignature(doubleType, T("b")));
      functions[i]->initialize(context, inputVariables);
    }
    
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
    
    progress = new ProgressionState(0, numGenerations, T("Generation"));
    for (size_t i = 0; i < numGenerations; ++i)
    {
      context.enterScope(T("Generation ") + String((int)i + 1));
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

      context.resultCallback(T("Tree ") + String((int)i), trees[i]->toString());
      context.resultCallback(T("Score Tree ") + String((int)i), Variable(results[i], doubleType));
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
