/*-----------------------------------------.---------------------------------.
| Filename: SampleExpressionTrajectories.h | A workunit for testing the      |
| Author  : Francis Maes                   | expression states               |
| Started : 12/10/2012 10:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_
# define LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp-ml/ExpressionDomain.h>
# include <lbcpp-ml/Domain.h>

namespace lbcpp
{

class ExpressionSearchProbabilitiesObjective : public Objective
{
public:
  ExpressionSearchProbabilitiesObjective(ExpressionDomainPtr expressionDomain, size_t maxSize, bool usePostfixNotation, const std::vector<size_t>& countPerArity)
    : expressionDomain(expressionDomain), maxSize(maxSize), usePostfixNotation(usePostfixNotation), countPerArity(countPerArity) {}

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    worst = 0.0; best = log2((double)maxSize); // objective : maximize entropy
    //worst = DBL_MAX; best = 0.0; // objective: minimize error between expectation
  }

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    enum {precision = 1000};
    DenseDoubleVectorPtr probabilities = normalizeProbabilities(object.staticCast<DenseDoubleVector>());
    DenseDoubleVectorPtr histogram = estimateHistogram(context, precision, probabilities->getValues());
    return histogram->computeEntropy();
  }

  
  double computeExpectation(const DenseDoubleVectorPtr& histogram) const
  {
    double res = 0.0;
    for (size_t i = 0; i < histogram->getNumValues(); ++i)
      res += (double)i * histogram->getValue(i);
    return res + 1.0;
  }

  DenseDoubleVectorPtr normalizeProbabilities(const DenseDoubleVectorPtr& probabilities) const
  {
    DenseDoubleVectorPtr res = probabilities->cloneAndCast<DenseDoubleVector>();
    double Z = 0.0;
    for (size_t i = 0; i < res->getNumValues(); ++i)
    {
      double p = juce::jlimit(0.0, 1.0, res->getValue(i));
      res->setValue(i, p);
      Z += p;
    }
    if (!Z)
      return DenseDoubleVectorPtr();

    res->multiplyByScalar(1.0 / Z);
    return res;  
  }

  SamplerPtr makeSampler(ExecutionContext& context, const std::vector<double>& probabilities) const
  {
    SearchActionCodeGeneratorPtr codeGenerator; // FIXME

    //ExpressionActionCodeGeneratorPtr codeGenerator = new ExpressionActionCodeGenerator(1);
    size_t numSymbols = expressionDomain->getNumSymbols();
    DenseDoubleVectorPtr parameters = new DenseDoubleVector(numSymbols * maxSize, 0.0);
    size_t n = probabilities.size() / 2;
    jassert(n == countPerArity.size() + 1);
    for (size_t step = 0; step < maxSize; ++step)
    {
      for (size_t index = 0; index < numSymbols; ++index)
      {
        ObjectPtr symbol = expressionDomain->getSymbol(index);
        double p;
        if (symbol)
        {
          size_t arity = expressionDomain->getSymbolArity(symbol);
          p = countPerArity[arity] ? lerp(probabilities, arity, step) / (double)countPerArity[arity] : 0.0;
        }
        else
          p = lerp(probabilities, n - 1, step);
        //size_t code = codeGenerator->getActionCode(expressionDomain, symbol, step, 0, maxSize);
        jassertfalse;
        size_t code = 0; // FIXME
        parameters->setValue(code, juce::jmax(-5.0, log(p)));
      }
    }


    SamplerPtr res = logLinearActionCodeSearchSampler(codeGenerator);
    int index = res->getClass()->findMemberVariable("parameters");
    res->setVariable(index, parameters);
    return res;
  }

  DenseDoubleVectorPtr estimateHistogram(ExecutionContext& context, size_t precision, const std::vector<double>& probabilities) const
  {
    DenseDoubleVectorPtr histogram(new DenseDoubleVector(maxSize, 0.0));
    double invZ = 1.0 / (double)precision;
    RandomGeneratorPtr random = context.getRandomGenerator();
    for (size_t i = 0; i < precision; ++i)
    {
      size_t size;
      if (usePostfixNotation)
        size = sampleSizeWithPostfixNotation(random, probabilities);
      else
        size = sampleSizeWithPrefixNotation(random, probabilities);
      jassert(size >= 1 && size <= maxSize);
      histogram->incrementValue(size - 1, invZ);
    }
    return histogram;
  }
  

protected:
  ExpressionDomainPtr expressionDomain;
  size_t maxSize;
  bool usePostfixNotation;

  std::vector<size_t> countPerArity;

  double lerp(const std::vector<double>& probabilities, size_t index, size_t currentSize) const
  {
    double k = juce::jlimit(0.0, 1.0, currentSize / (double)(maxSize - 1));
    double a = probabilities[index];
    double b = probabilities[index + probabilities.size() / 2];
    return a + k * (b - a);
  }

  size_t sampleSizeWithPrefixNotation(RandomGeneratorPtr random, const std::vector<double>& probabilities) const
  {
    size_t numLeafs = 1;
    size_t size = 0;
    size_t n = probabilities.size() / 2;
    jassert(n == countPerArity.size() + 1);
    while (numLeafs > 0)
    {
      jassert(maxSize >= size + numLeafs);
      size_t maxArity = maxSize - size - numLeafs;
      size_t arity;
      std::vector<double> p(countPerArity.size());
      for (size_t i = 0; i < p.size(); ++i)
        if (!countPerArity[i] || i > maxArity)
          p[i] = 0.0;
        else
          p[i] = lerp(probabilities, i, size);
      arity = random->sampleWithProbabilities(p);
      numLeafs += arity - 1;
      ++size;
    }
    return size;
  }

  size_t sampleSizeWithPostfixNotation(RandomGeneratorPtr random, const std::vector<double>& probabilities) const
  {
    size_t stackSize = 0;
    size_t size = 0;
    size_t maxFunctionArity = countPerArity.size() - 1;
    size_t n = probabilities.size() / 2;
    jassert(n == countPerArity.size() + 1);
    while (size < maxSize)
    {
      std::vector<double> p(n);
      size_t maxArity = (size_t)juce::jmin((int)maxFunctionArity, (int)stackSize); // cannot apply a n-ary operator if there are no n elements on the stack
      size_t minArity = (size_t)juce::jlimit(0, (int)maxFunctionArity, (int)stackSize - (int)((maxSize - size - 1) * (maxFunctionArity - 1)));
      bool isYieldable = (stackSize == 1);
  
      for (size_t i = 0; i < countPerArity.size(); ++i)
        if (!countPerArity[i] || i < minArity || i > maxArity)
          p[i] = 0.0;
        else
          p[i] = lerp(probabilities, i, size);
      if (!isYieldable)
        p.back() = 0.0;
      else
        p.back() = lerp(probabilities, n-1, size);

      size_t action = random->sampleWithProbabilities(p);
      if (action == n - 1)
        break; // yielded
      stackSize += 1 - action;
      ++size;
    }
    return size;
  }
};

typedef ReferenceCountedObjectPtr<ExpressionSearchProbabilitiesObjective> ExpressionSearchProbabilitiesObjectivePtr;

class ExpressionSearchProbabilitiesProblem : public NewProblem
{
public:
  ExpressionSearchProbabilitiesProblem(ExpressionDomainPtr expressionDomain, size_t maxSize, bool usePostfixNotation)
    : expressionDomain(expressionDomain), maxSize(maxSize), usePostfixNotation(usePostfixNotation)
    {initialize(defaultExecutionContext());}
  
  virtual void initialize(ExecutionContext& context)
  {
    std::vector<size_t> countPerArity;
    countPerArity.push_back(expressionDomain->getNumConstants() + expressionDomain->getNumInputs() + expressionDomain->getNumActiveVariables());
    size_t maxArity = expressionDomain->getMaxFunctionArity();
    for (size_t arity = 1; arity <= maxArity; ++arity)
    {
      size_t count = 0;
      for (size_t j = 0; j < expressionDomain->getNumFunctions(); ++j)
        if (expressionDomain->getFunction(j)->getNumInputs() == arity)
          ++count;
      countPerArity.push_back(count);
    }
    size_t n = countPerArity.size() + 1;

    DenseDoubleVectorPtr startingSolution = new DenseDoubleVector(n, 1.0);
    for (size_t i = 0; i < countPerArity.size(); ++i)
      startingSolution->setValue(i, countPerArity[i]);
    startingSolution->setValue(n-1, 1.0);
    startingSolution->multiplyByScalar(1.0 / startingSolution->l1norm());
    startingSolution->resize(2 * n);
    for (size_t i = 0; i < n; ++i)
      startingSolution->setValue(i + n, startingSolution->getValue(i));
    setInitialGuess(startingSolution);

    setDomain(new ContinuousDomain(std::vector< std::pair<double, double> >(n * 2, std::make_pair(0.0, 1.0))));
    addObjective(new ExpressionSearchProbabilitiesObjective(expressionDomain, maxSize, usePostfixNotation, countPerArity));
  }

protected:
  ExpressionDomainPtr expressionDomain;
  size_t maxSize;
  bool usePostfixNotation;
};

typedef ReferenceCountedObjectPtr<ExpressionSearchProbabilitiesProblem> ExpressionSearchProbabilitiesProblemPtr;

class SampleExpressionTrajectories : public WorkUnit
{
public:
  SampleExpressionTrajectories() : numExpressions(1000), maxExpressionSize(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!problem)
    {
      context.errorCallback("No problem defined");
      return false;
    }
    ExpressionDomainPtr domain = problem->getDomain();

    context.informationCallback(domain->toShortString());
    SamplerPtr prefixSampler = optimizeDistribution(context, false);
    SamplerPtr postfixSampler = optimizeDistribution(context, true);

    context.resultCallback("prefixSampler", prefixSampler);
    context.resultCallback("postfixSampler", postfixSampler);

    sampleTrajectories(context, "prefix-initial", prefixExpressionState(domain, maxExpressionSize), randomSearchSampler());
    sampleTrajectories(context, "postfix-initial", postfixExpressionState(domain, maxExpressionSize), randomSearchSampler());
    sampleTrajectories(context, "prefix-optimized", prefixExpressionState(domain, maxExpressionSize), prefixSampler);
    sampleTrajectories(context, "postfix-optimized", postfixExpressionState(domain, maxExpressionSize), postfixSampler);
    //sampleTrajectories(context, "typed-postfix", typedPostfixExpressionState(domain, maxExpressionSize));


    return true;
  }

protected:
  friend class SampleExpressionTrajectoriesClass;

  ProblemPtr problem;
  size_t numExpressions;
  size_t maxExpressionSize;

  SamplerPtr optimizeDistribution(ExecutionContext& context, bool usePostfixNotation)
  {
    context.enterScope(String("Optimizing probabilities with ") + (usePostfixNotation ? "postfix" : "prefix") + " notation");
    ExpressionDomainPtr domain = problem->getDomain().staticCast<ExpressionDomain>();
    ExpressionSearchProbabilitiesProblemPtr problem = new ExpressionSearchProbabilitiesProblem(this->problem->getDomain(), maxExpressionSize, usePostfixNotation);
    ExpressionSearchProbabilitiesObjectivePtr objective = problem->getObjective(0).staticCast<ExpressionSearchProbabilitiesObjective>();
    SolverPtr solver = crossEntropySolver(diagonalGaussianDistributionSampler(), 100, 30, 20, false);
    solver->setVerbosity(verbosityAll);
    ParetoFrontPtr solutions = new ParetoFront();
    solver->solve(context, problem, fillParetoFrontSolverCallback(solutions));
    DenseDoubleVectorPtr probabilities = objective->normalizeProbabilities(solutions->getSolution(0).staticCast<DenseDoubleVector>());
    context.resultCallback("initial", problem->getInitialGuess());
    context.resultCallback("optimized", probabilities);
    context.leaveScope();
    computeHistogramForProbabilities(context, "initial", problem->getInitialGuess(), usePostfixNotation);
    computeHistogramForProbabilities(context, "optimized", probabilities, usePostfixNotation);

    return objective->makeSampler(context, probabilities->getValues());
  }

  void computeHistogramForProbabilities(ExecutionContext& context, const String& name, const DenseDoubleVectorPtr& probabilities, bool usePostfixNotation)
  {
    ExpressionDomainPtr domain = problem->getDomain().staticCast<ExpressionDomain>();
    ExpressionSearchProbabilitiesProblemPtr test = new ExpressionSearchProbabilitiesProblem(problem->getDomain(), maxExpressionSize, usePostfixNotation);
    ExpressionSearchProbabilitiesObjectivePtr objective = test->getObjective(0).staticCast<ExpressionSearchProbabilitiesObjective>();

    DenseDoubleVectorPtr histo = objective->estimateHistogram(context, numExpressions, probabilities->getValues());
    context.enterScope(name + " -> " + String(histo->computeEntropy()));
    for (size_t i = 0; i < histo->getNumValues(); ++i)
    {
      context.enterScope(String((int)i+1));
      context.resultCallback("i", i+1);
      context.resultCallback("p", histo->getValue(i));
      context.leaveScope();
    }
    context.leaveScope();
  }

  void sampleTrajectories(ExecutionContext& context, const String& name, ExpressionStatePtr initialState, SamplerPtr sampler)
  {
    std::vector<size_t> countsPerSize;
    sampler->initialize(context, new SearchDomain(initialState));

    context.enterScope(name);
    context.resultCallback("initialState", initialState);
    context.enterScope("Sampling");
    for (size_t i = 0; i < numExpressions; ++i)
    {
      SearchTrajectoryPtr trajectory = sampler->sample(context).staticCast<SearchTrajectory>();
      ExpressionPtr expression = trajectory->getFinalState()->getConstructedObject().staticCast<Expression>();
      if (i < 20)
      {
        context.informationCallback(T("Trajectory: ") + trajectory->toShortString());
        context.informationCallback(T(" => ") + expression->toShortString());
      }
      size_t size = expression->getTreeSize();
      jassert(size > 0);
      if (countsPerSize.size() <= size)
        countsPerSize.resize(size + 1, 0);
      countsPerSize[size]++;
      context.progressCallback(new ProgressionState(i+1, numExpressions, "Expressions"));
    }
    context.leaveScope();

    context.enterScope("Size Histogram");
    for (size_t i = 1; i < countsPerSize.size(); ++i)
    {
      context.enterScope("Size " + String((int)i));
      context.resultCallback("size", i);
      context.resultCallback("probability", countsPerSize[i] / (double)numExpressions);
      context.leaveScope();
    }
    context.leaveScope();

    context.leaveScope();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_SAMPLE_EXPRESSION_TRAJECTORIES_H_
