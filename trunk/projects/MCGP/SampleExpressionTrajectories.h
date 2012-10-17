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

class ExpressionSearchProbabilitiesProblem : public ContinuousProblem
{
public:
  ExpressionSearchProbabilitiesProblem(ExpressionDomainPtr domain, size_t maxSize, bool usePostfixNotation)
    : maxSize(maxSize), usePostfixNotation(usePostfixNotation)
  {
    std::set<size_t> aritySet;
    aritySet.insert(0); // variables, constants
    for (size_t i = 0; i < domain->getNumFunctions(); ++i)
      aritySet.insert(domain->getFunction(i)->getNumInputs());
    
    arities.reserve(aritySet.size());
    for (std::set<size_t>::const_iterator it = aritySet.begin(); it != aritySet.end(); ++it)
      arities.push_back(*it);
    size_t n = arities.size();
    if (usePostfixNotation)
      ++n;

    startingSolution = new DenseDoubleVector(n, 0.0);
    startingSolution->setValue(0, (double)(domain->getNumConstants() + domain->getNumInputs() + domain->getNumActiveVariables()));
    for (size_t i = 1; i < arities.size(); ++i)
    {
      size_t count = 0;
      for (size_t j = 0; j < domain->getNumFunctions(); ++j)
        if (domain->getFunction(j)->getNumInputs() == arities[i])
          ++count;
      startingSolution->setValue(i, (double)count);
    }
    if (usePostfixNotation)
      startingSolution->setValue(arities.size(), 1.0); // probability of yield
    startingSolution->multiplyByScalar(1.0 / startingSolution->l1norm());

    this->domain = new ContinuousDomain(std::vector< std::pair<double, double> >(n, std::make_pair(0.0, 1.0)));
    //this->limits = new FitnessLimits(std::vector< std::pair<double, double> >(1, std::make_pair(0.0, log2((double)maxSize)))); // objective: maximize entropy
    this->limits = new FitnessLimits(std::vector< std::pair<double, double> >(1, std::make_pair(DBL_MAX, 0.0))); // objective: minimize error between expectation
  }

  virtual FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    enum {precision = 10000};
    DenseDoubleVectorPtr probabilities = normalizeProbabilities(object.staticCast<DenseDoubleVector>());
    DenseDoubleVectorPtr histogram = estimateHistogram(context, precision, probabilities->getValues());
    return new Fitness(fabs(computeExpectation(histogram) - 15), limits);
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
  
  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return startingSolution;}

protected:
  std::vector<size_t> arities;
  DenseDoubleVectorPtr startingSolution;
  size_t maxSize;
  bool usePostfixNotation;

  size_t sampleSizeWithPrefixNotation(RandomGeneratorPtr random, const std::vector<double>& probabilities) const
  {
    jassert(probabilities.size() == arities.size());
    size_t numLeafs = 1;
    size_t size = 0;
    while (numLeafs > 0)
    {
      jassert(maxSize >= size + numLeafs);
      size_t maxArity = maxSize - size - numLeafs;
      size_t arity;
      if (maxArity < arities.back())
      {
        std::vector<double> p(probabilities);
        for (size_t i = 0; i < p.size(); ++i)
          if (arities[i] > maxArity)
            p[i] = 0.0;
        arity = arities[random->sampleWithProbabilities(p)];
      }
      else
        arity = arities[random->sampleWithNormalizedProbabilities(probabilities)];
      numLeafs += arity - 1;
      ++size;
    }
    return size;
  }

  size_t sampleSizeWithPostfixNotation(RandomGeneratorPtr random, const std::vector<double>& probabilities) const
  {
    jassert(probabilities.size() == arities.size() + 1);
    size_t stackSize = 0;
    size_t size = 0;
    size_t maxFunctionArity = arities.back();
    while (size < maxSize)
    {
      std::vector<double> p(probabilities);
      size_t maxArity = (size_t)juce::jmin((int)maxFunctionArity, (int)stackSize); // cannot apply a n-ary operator if there are no n elements on the stack
      size_t minArity = (size_t)juce::jlimit(0, (int)maxFunctionArity, (int)stackSize - (int)((maxSize - size - 1) * (maxFunctionArity - 1)));
      bool isYieldable = (stackSize == 1);
  
      for (size_t i = 0; i < arities.size(); ++i)
      {
        size_t arity = arities[i];
        if (arity < minArity || arity > maxArity)
          p[i] = 0.0;
      }
      if (!isYieldable)
        p.back() = 0.0;

      size_t action = random->sampleWithProbabilities(p);
      if (action == arities.size())
        break; // yielded
      stackSize += 1 - arities[action];
      ++size;
    }
    return size;
  }
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
    optimizeDistribution(context, false);
    optimizeDistribution(context, true);
    sampleTrajectories(context, "prefix", prefixExpressionState(domain, maxExpressionSize));
    sampleTrajectories(context, "postfix", postfixExpressionState(domain, maxExpressionSize));
    sampleTrajectories(context, "typed-postfix", typedPostfixExpressionState(domain, maxExpressionSize));
    return true;
  }

protected:
  friend class SampleExpressionTrajectoriesClass;

  ExpressionProblemPtr problem;
  size_t numExpressions;
  size_t maxExpressionSize;

  DenseDoubleVectorPtr optimizeDistribution(ExecutionContext& context, bool usePostfixNotation)
  {
    context.enterScope(String("Optimizing probabilities with ") + (usePostfixNotation ? "postfix" : "prefix") + " notation");
    ExpressionDomainPtr domain = problem->getDomain().staticCast<ExpressionDomain>();
    ExpressionSearchProbabilitiesProblemPtr problem = new ExpressionSearchProbabilitiesProblem(this->problem->getDomain(), maxExpressionSize, usePostfixNotation);
    SolverPtr solver = crossEntropyOptimizer(diagonalGaussianDistributionSampler(), 100, 30, 20, false);
    SolutionContainerPtr solutions = solver->optimize(context, problem, ObjectPtr(), Solver::verbosityAll);
    DenseDoubleVectorPtr probabilities = problem->normalizeProbabilities(solutions->getSolution(0).staticCast<DenseDoubleVector>());
    context.resultCallback("initial", problem->proposeStartingSolution(context));
    context.resultCallback("optimized", probabilities);
    context.leaveScope();
    computeHistogramForProbabilities(context, "initial", problem->proposeStartingSolution(context), usePostfixNotation);
    computeHistogramForProbabilities(context, "optimized", probabilities, usePostfixNotation);
    return probabilities;
  }

  void computeHistogramForProbabilities(ExecutionContext& context, const String& name, const DenseDoubleVectorPtr& probabilities, bool usePostfixNotation)
  {
    ExpressionDomainPtr domain = problem->getDomain().staticCast<ExpressionDomain>();
    ExpressionSearchProbabilitiesProblemPtr test = new ExpressionSearchProbabilitiesProblem(problem->getDomain(), maxExpressionSize, usePostfixNotation);

    DenseDoubleVectorPtr histo = test->estimateHistogram(context, numExpressions, probabilities->getValues());
    context.enterScope(name + " -> " + String(test->computeExpectation(histo)));
    for (size_t i = 0; i < histo->getNumValues(); ++i)
    {
      context.enterScope(String((int)i+1));
      context.resultCallback("i", i+1);
      context.resultCallback("p", histo->getValue(i));
      context.leaveScope();
    }
    context.leaveScope();
  }

  void sampleTrajectories(ExecutionContext& context, const String& name, ExpressionStatePtr initialState)
  {
    std::vector<size_t> countsPerSize;
    SamplerPtr sampler = randomSearchSampler();
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
