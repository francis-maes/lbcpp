/*-----------------------------------------.---------------------------------.
| Filename: ClassificationSandBox.h        | Classification SandBox          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 10:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
# define LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/Expression.h>
# include <lbcpp-ml/SplittingCriterion.h>
# include <algorithm>

namespace lbcpp
{

/////////////////////////////////////////////////////

class VectorDomain : public Domain
{
public:
  VectorDomain(DomainPtr elementsDomain = DomainPtr())
    : elementsDomain(elementsDomain) {}

  DomainPtr getElementsDomain() const
    {return elementsDomain;}

protected:
  friend class VectorDomainClass;

  DomainPtr elementsDomain;
};

extern DomainPtr vectorDomain(DomainPtr elementsDomain);

/////////////////////////////////////////////////////

class ScalarExpressionVectorSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    ExpressionDomainPtr expressionDomain = domain.staticCast<VectorDomain>()->getElementsDomain().staticCast<ExpressionDomain>();
    expressions = vector(expressionClass, 0);
    for (size_t i = 0; i < expressionDomain->getNumInputs(); ++i)
    {
      VariableExpressionPtr input = expressionDomain->getInput(i);
      if (input->getType()->isConvertibleToDouble())
        expressions->append(input);
    }
  }

  virtual ObjectPtr sample(ExecutionContext& context) const 
    {return expressions;}

private:
  OVectorPtr expressions;
};

class SubsetVectorSampler : public Sampler
{
public:
  SubsetVectorSampler(SamplerPtr vectorSampler, size_t subsetSize)
    : vectorSampler(vectorSampler), subsetSize(subsetSize) {}
  SubsetVectorSampler() : subsetSize(0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {vectorSampler->initialize(context, domain);}

  virtual ObjectPtr sample(ExecutionContext& context) const 
  {
    VectorPtr elements = vectorSampler->sample(context);
    if (elements->getNumElements() <= subsetSize)
      return elements;
    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(elements->getNumElements(), order);
    VectorPtr res = vector(elements->getElementsType(), subsetSize);
    for (size_t i = 0; i < subsetSize; ++i)
      res->setElement(i, elements->getElement(order[i]));
    return res;
  }

protected:
  friend class SubsetVectorSamplerClass;

  SamplerPtr vectorSampler;
  size_t subsetSize;
};

/////////////////////////////////////////////////////

class ExhaustiveConditionLearner : public Solver
{
public:
  ExhaustiveConditionLearner(SamplerPtr expressionsSampler)
    : expressionsSampler(expressionsSampler) {}
  ExhaustiveConditionLearner() {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    Solver::startSolver(context, problem, callback, startingSolution);
    expressionsSampler->initialize(context, vectorDomain(problem->getDomain()));
  }

  virtual void runSolver(ExecutionContext& context)
  {
    SplittingCriterionPtr splittingCriterion = problem->getObjective(0).staticCast<SplittingCriterion>();
    OVectorPtr expressions = expressionsSampler->sample(context).staticCast<OVector>();
    for (size_t i = 0; i < expressions->getNumElements(); ++i)
    {
      ExpressionPtr expression = expressions->getElement(i).staticCast<Expression>();
      std::pair<double, ExpressionPtr> p = computeCriterionWithEventualStump(context, splittingCriterion, expression);
      callback->solutionEvaluated(context, refCountedPointerFromThis(this), p.second, new Fitness(p.first, problem->getFitnessLimits()));
    }
  }

protected:
  friend class ExhaustiveConditionLearnerClass;

  SamplerPtr expressionsSampler;
  
  struct SortDoubleValuesOperator
  {
    static double transformIntoValidNumber(double input)
      {return input;}

    bool operator()(const std::pair<size_t, double>& a, const std::pair<size_t, double>& b) const
      {return a.second == b.second ? a.first < b.first : a.second < b.second;}
  };

  static SparseDoubleVectorPtr sortDoubleValues(const DataVectorPtr& data)
  {
    size_t n = data->size();
    SparseDoubleVectorPtr res = new SparseDoubleVector(n);
    std::vector< std::pair<size_t, double> >& v = res->getValuesVector();
  
    bool isDouble = (data->getElementsType() == doubleClass);
    for (DataVector::const_iterator it = data->begin(); it != data->end(); ++it)
    {
      double value = isDouble ? it.getRawDouble() : it.getRawObject()->toDouble();
      if (value != DVector::missingValue)
        v.push_back(std::make_pair(it.getIndex(), value));
    }
    std::sort(v.begin(), v.end(), SortDoubleValuesOperator());
    return res;
  }

  std::pair<double, ExpressionPtr> computeCriterionWithEventualStump(ExecutionContext& context, SplittingCriterionPtr splittingCriterion, const ExpressionPtr& booleanOrScalar)
  {
     if (booleanOrScalar->getType() == booleanClass)
      return std::make_pair(splittingCriterion->evaluate(context, booleanOrScalar), booleanOrScalar);
    else
    {
      jassert(booleanOrScalar->getType()->isConvertibleToDouble());
      double criterion;
      DataVectorPtr values = splittingCriterion->computePredictions(context, booleanOrScalar);
      SparseDoubleVectorPtr sortedDoubleValues = sortDoubleValues(values); // FIXME: cache sortedDoubleValues when possible
      double threshold = findBestThreshold(context, splittingCriterion, booleanOrScalar, sortedDoubleValues, criterion);
      return std::make_pair(criterion, new FunctionExpression(stumpFunction(threshold), booleanOrScalar));
    }
  }
  
  double findBestThreshold(ExecutionContext& context, SplittingCriterionPtr splittingCriterion, const ExpressionPtr& scalar, const SparseDoubleVectorPtr& sortedDoubleValues, double& bestScore)
  {
    splittingCriterion->setPredictions(DataVector::createConstant(splittingCriterion->getIndices(), new Boolean(false)));
    splittingCriterion->ensureIsUpToDate();

    if (sortedDoubleValues->getNumValues() == 0)
    {
      bestScore = splittingCriterion->computeCriterion();
      return 0.0;
    }

    bestScore = -DBL_MAX;
    std::vector<double> bestThresholds;

    if (verbosity >= verbosityDetailed)
      context.enterScope("Find best threshold for node " + scalar->toShortString());

    size_t n = sortedDoubleValues->getNumValues();
    double previousThreshold = sortedDoubleValues->getValue(n - 1).second;
    for (int i = (int)n - 1; i >= 0; --i)
    {
      size_t index = sortedDoubleValues->getValue(i).first;
      double threshold = sortedDoubleValues->getValue(i).second;

      jassert(threshold <= previousThreshold);
      if (threshold < previousThreshold)
      {
        double e = splittingCriterion->computeCriterion();

        if (verbosity >= verbosityAll)
        {
          context.enterScope("Iteration " + string((int)i));
          context.resultCallback("threshold", (threshold + previousThreshold) / 2.0);
          context.resultCallback("splittingCriterion", e);
          context.leaveScope();
        }

        if (e >= bestScore)
        {
          if (e > bestScore)
          {
            bestThresholds.clear();
            bestScore = e;
          }
          bestThresholds.push_back((threshold + previousThreshold) / 2.0);
        }
        previousThreshold = threshold;
      }
      splittingCriterion->flipPrediction(index);
    }

    if (verbosity >= verbosityDetailed)
      context.leaveScope(PairPtr(new Pair(new Double(bestThresholds.size() ? bestThresholds[0] : 0.0), new Double(bestScore))));

    return bestThresholds.size() ? bestThresholds[bestThresholds.size() / 2] : 0; // median value
  }
};


/////////////////////////////////////////////////////

class TreeLearner : public Solver
{
public:
  TreeLearner(SplittingCriterionPtr splittingCriterion, SolverPtr conditionLearner, size_t minExamplesToSplit, size_t maxDepth)
    : splittingCriterion(splittingCriterion), conditionLearner(conditionLearner), minExamplesToSplit(minExamplesToSplit), maxDepth(maxDepth) {}
  TreeLearner() {}

  virtual void runSolver(ExecutionContext& context)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    TablePtr data = objective->getData();

    ExpressionPtr res = makeTreeScope(context, objective, objective->getIndices(), 1);
    if (verbosity >= verbosityProgressAndResult)
    {
      size_t treeDepth = 0;
      ScalarVariableStatisticsPtr nodeSizeStats = new ScalarVariableStatistics(T("nodeSize"));
      size_t numNodes = getNumTestNodes(res, 0, treeDepth, nodeSizeStats);
      context.resultCallback(T("treeDepth"), treeDepth);
      context.resultCallback(T("treeSize"), numNodes);
      context.resultCallback(T("conditionSize"), nodeSizeStats);
      context.resultCallback(T("meanConditionSize"), nodeSizeStats->getMean());
      context.informationCallback(T("Tree depth = ") + string((int)treeDepth) + T(" size = ") + string((int)numNodes));
    }
    evaluate(context, res);
  }

protected:
  friend class TreeLearnerClass;

  SplittingCriterionPtr splittingCriterion;
  SolverPtr conditionLearner;
  size_t minExamplesToSplit;
  size_t maxDepth;

  bool isConstant(const VectorPtr& data, const IndexSetPtr& indices) const
  {
    if (indices->size() <= 1)
      return true;
    IndexSet::const_iterator it = indices->begin();
    ObjectPtr value = data->getElement(*it);
    for (++it; it != indices->end(); ++it)
      if (!Object::equals(value, data->getElement(*it)))
        return false;
    return true;
  }
  
  ExpressionPtr makeTreeScope(ExecutionContext& context, const SupervisedLearningObjectivePtr& objective, const IndexSetPtr& indices, size_t depth)
  {
    if (verbosity >= verbosityDetailed)
      context.enterScope(T("Make tree with ") + string((int)indices->size()) + " examples");
    ExpressionPtr res = makeTree(context, objective, indices, depth);
    if (verbosity >= verbosityDetailed)
      context.leaveScope();
    return res;
  }

  ExpressionPtr makeTree(ExecutionContext& context, const SupervisedLearningObjectivePtr& objective, const IndexSetPtr& indices, size_t depth)
  {
    // min examples and max depth conditions to make a leaf
    if ((indices->size() < minExamplesToSplit) || (maxDepth && depth == maxDepth) || isConstant(objective->getSupervisions(), indices))
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    // launch condition learner
    splittingCriterion->configure(objective->getData(), objective->getSupervision(), DenseDoubleVectorPtr(), indices);
    ProblemPtr conditionProblem = new Problem(problem->getDomain(), splittingCriterion);
    ExpressionPtr conditionNode;
    FitnessPtr conditionFitness;
    conditionLearner->solve(context, conditionProblem, storeBestSolverCallback(*(ObjectPtr* )&conditionNode, conditionFitness));

    // check condition and update importance values
    if (!conditionNode || conditionNode.isInstanceOf<ConstantExpression>())
      return new ConstantExpression(splittingCriterion->computeVote(indices));
    conditionNode->addImportance(conditionFitness->getValue(0) * indices->size() / objective->getNumSamples());
    if (verbosity >= verbosityDetailed)
      context.informationCallback(conditionNode->toShortString() + T(" [") + string(conditionNode->getSubNode(0)->getImportance()) + T("]"));

    // otherwise split examples...
    DataVectorPtr conditionValues = conditionNode->compute(context, objective->getData(), indices);
    IndexSetPtr failureExamples, successExamples, missingExamples;
    TestExpression::dispatchIndices(conditionValues, failureExamples, successExamples, missingExamples);

    if (failureExamples->size() == indices->size() || successExamples->size() == indices->size() || missingExamples->size() == indices->size())
      return new ConstantExpression(splittingCriterion->computeVote(indices));

    // ...call recursively
    if (verbosity >= verbosityDetailed)
      context.enterScope(conditionNode->toShortString());
    ExpressionPtr failureNode = makeTreeScope(context, objective, failureExamples, depth + 1);
    ExpressionPtr successNode = makeTreeScope(context, objective, successExamples, depth + 1);
    ExpressionPtr missingNode = makeTreeScope(context, objective, missingExamples, depth + 1);
    if (verbosity >= verbosityDetailed)
      context.leaveScope();

    // and build a test node.
    return new TestExpression(conditionNode, failureNode, successNode, missingNode);
  }

  size_t getNumTestNodes(const ExpressionPtr& node, size_t depth, size_t& maxDepth, ScalarVariableStatisticsPtr nodeSizeStats) const
  {
    if (depth > maxDepth)
      maxDepth = depth;
    size_t res = 0;
    TestExpressionPtr testNode = node.dynamicCast<TestExpression>();
    if (testNode)
    {
      nodeSizeStats->push(testNode->getCondition()->getTreeSize());
      ++res;
      res += getNumTestNodes(testNode->getFailure(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getSuccess(), depth + 1, maxDepth, nodeSizeStats);
      res += getNumTestNodes(testNode->getMissing(), depth + 1, maxDepth, nodeSizeStats);
    }
    return res;
  }
};

/////////////////////////////////////////////////////

extern SamplerPtr scalarExpressionVectorSampler();
extern SamplerPtr subsetVectorSampler(SamplerPtr vectorSampler, size_t subsetSize);

extern SolverPtr exhaustiveConditionLearner(SamplerPtr expressionsSampler);
extern SolverPtr treeLearner(SplittingCriterionPtr splittingCriterion, SolverPtr conditionLearner, size_t minExamplesToSplit = 2, size_t maxDepth = 0);


extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp


class ClassificationSandBox : public WorkUnit
{
public:
  ClassificationSandBox() : maxExamples(0), verbosity(0) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);

    // load dataset and display information
    std::vector<VariableExpressionPtr> inputs;
    VariableExpressionPtr supervision;
    TablePtr dataset = loadDataFile(context, dataFile, inputs, supervision);
    if (!dataset || !inputs.size() || !supervision)
      return new Boolean(false);

    size_t numVariables = inputs.size();
    size_t numExamples = dataset->getNumRows();
    size_t numLabels = supervision->getType().staticCast<Enumeration>()->getNumElements();
    context.informationCallback(string((int)numExamples) + T(" examples, ") +
                                string((int)numVariables) + T(" variables, ") +
                                string((int)numLabels) + T(" labels"));
    context.resultCallback("dataset", dataset);

    // make train/test split
    size_t numTrainingSamples = getNumTrainingSamples(dataFile);
    dataset = dataset->randomize(context);
    TablePtr trainingData = dataset->range(0, numTrainingSamples);
    TablePtr testingData = dataset->invRange(0, numTrainingSamples);
    context.informationCallback(string((int)trainingData->getNumRows()) + " training examples, " + string((int)testingData->getNumRows()) + " testing examples");

    // make problem
    ProblemPtr problem = makeProblem(context, inputs, supervision, trainingData, testingData);
    context.resultCallback("problem", problem);

    // make learner
    SamplerPtr expressionVectorSampler = subsetVectorSampler(scalarExpressionVectorSampler(), (size_t)(sqrt((double)numVariables) + 0.5));
    SolverPtr conditionLearner = exhaustiveConditionLearner(expressionVectorSampler);
    conditionLearner->setVerbosity((SolverVerbosity)verbosity);
    SolverPtr learner = treeLearner(informationGainSplittingCriterion(true), conditionLearner);
    learner->setVerbosity((SolverVerbosity)verbosity);

    // learn
    ParetoFrontPtr front = new ParetoFront(problem->getFitnessLimits());
    SolverCallbackPtr callback = fillParetoFrontSolverCallback(front);
    learner->solve(context, problem, callback);
    context.resultCallback("front", front);

    // evaluate

    return new Boolean(true);
  }
  
private:
  friend class ClassificationSandBoxClass;

  juce::File dataFile;
  size_t maxExamples;
  size_t verbosity;

  TablePtr loadDataFile(ExecutionContext& context, const juce::File& file, std::vector<VariableExpressionPtr>& inputs, VariableExpressionPtr& supervision)
  {
    context.enterScope(T("Loading ") + file.getFileName());
    TablePtr res = Object::createFromFile(context, file).staticCast<Table>();
    context.leaveScope(res ? res->getNumRows() : 0);
    if (!res)
      return TablePtr();

    // we take all numerical attributes as input and take the latest categorical attribute as supervision
    for (size_t i = 0; i < res->getNumColumns(); ++i)
    {
      if (res->getType(i)->inheritsFrom(doubleClass))
        inputs.push_back(res->getKey(i));
      else if (res->getType(i)->inheritsFrom(enumValueClass))
        supervision = res->getKey(i);
    }
    return res;
  }

  size_t getNumTrainingSamples(const juce::File& dataFile) const
  {
    if (dataFile.getFileName() == T("waveform.jdb"))
      return 300;
    jassertfalse;
    return 0;
  }

  ProblemPtr makeProblem(ExecutionContext& context, const std::vector<VariableExpressionPtr>& inputs, const VariableExpressionPtr& supervision, const TablePtr& trainingData, const TablePtr& testingData)
  {
    ProblemPtr res = new Problem();
    ExpressionDomainPtr domain = new ExpressionDomain();
    domain->addInputs(inputs);
    domain->setSupervision(supervision);

    res->setDomain(domain);
    res->addObjective(multiClassAccuracyObjective(trainingData, supervision));
    // todo: addValidationObjective(...)
    return res;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
