/*-----------------------------------------.---------------------------------.
| Filename: TreeGPSamplers.h               | Tree based Genetic Programming  |
| Author  : Francis Maes                   | Implementation based on         |
| Started : 20/10/2012 22:33               |  samplers                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_TREE_GP_SAMPLERS_H_
# define LBCPP_MCGP_TREE_GP_SAMPLERS_H_

# include <lbcpp-ml/SolutionComparator.h>
# include <lbcpp-ml/ExpressionSampler.h>
# include <lbcpp-ml/Solver.h>

namespace lbcpp
{

class TournamentSampler : public Sampler
{
public:
  TournamentSampler(SolutionComparatorPtr comparator, size_t tournamentSize)
    : comparator(comparator), tournamentSize(tournamentSize) {}
  TournamentSampler() : tournamentSize(0) {}

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    jassert(solutions);
    RandomGeneratorPtr random = context.getRandomGenerator();
    jassert(tournamentSize > 0);
    size_t n = solutions->getNumSolutions();
    size_t best = random->sampleSize(n);
    for (size_t i = 1; i < tournamentSize; ++i)
    {
      size_t candidate = random->sampleSize(n);
      if (comparator->compare(candidate, best) < 0)
        best = candidate;
    }
    return solutions->getSolution(best);
  }
  
  virtual void learn(ExecutionContext& context, const SolutionVectorPtr& solutions)
  {
    this->solutions = solutions;
    comparator->initialize(solutions);
  }

protected:
  friend class TournamentSamplerClass;

  SolutionComparatorPtr comparator;
  size_t tournamentSize;
  SolutionVectorPtr solutions;
};

class PerturbatorSampler : public DecoratorSampler
{
public:
  PerturbatorSampler(SamplerPtr inputSampler, PerturbatorPtr perturbator, double probability)
    : DecoratorSampler(inputSampler), perturbator(perturbator), probability(probability) {}
  PerturbatorSampler() : probability(0.0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DecoratorSampler::initialize(context, domain);
    perturbator->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    ObjectPtr res = DecoratorSampler::sample(context);
    if (context.getRandomGenerator()->sampleBool(probability))
      res = perturbator->sample(context, res);
    return res;
  }

protected:
  friend class PerturbatorSamplerClass;

  PerturbatorPtr perturbator;
  double probability;
};

class BinaryPerturbatorSampler : public DecoratorSampler
{
public:
  BinaryPerturbatorSampler(SamplerPtr inputSampler, BinaryPerturbatorPtr perturbator, double probability)
    : DecoratorSampler(inputSampler), perturbator(perturbator), probability(probability) {}
  BinaryPerturbatorSampler() : probability(0.0) {}
  
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    DecoratorSampler::initialize(context, domain);
    perturbator->initialize(context, domain);
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    if (nextObject)
    {
      ObjectPtr res = nextObject;
      const_cast<BinaryPerturbatorSampler* >(this)->nextObject = ObjectPtr();
      return res;
    }
    else
    { 
      ObjectPtr input1 = DecoratorSampler::sample(context);
      ObjectPtr input2 = DecoratorSampler::sample(context);
      std::pair<ObjectPtr, ObjectPtr> pair;
      if (context.getRandomGenerator()->sampleBool(probability))
        pair = perturbator->samplePair(context, input1, input2);
      else
        pair = std::make_pair(input1, input2);
      const_cast<BinaryPerturbatorSampler* >(this)->nextObject = pair.second;
      return pair.first;
    }
  }

protected:
  friend class BinaryPerturbatorSamplerClass;

  BinaryPerturbatorPtr perturbator;
  double probability;

  ObjectPtr nextObject;
};

////////////////////////////////////////////////////////////////////////////
/*
class GeneratePerturbationExamples : public Solver
{
public:
  GeneratePerturbationExamples(SamplerPtr sampler, PerturbatorPtr perturbator, size_t count)
    : sampler(sampler), perturbator(perturbator), count(count) {}

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    sampler->initialize(context, problem->getDomain());
    perturbator->initialize(context, problem->getDomain());
  }

  virtual void runSolver(ExecutionContext& context)
  {
    File outputFile = context.getFile("examples.txt");
    if (outputFile.exists())
      outputFile.deleteFile();
    juce::OutputStream* ostr = outputFile.createOutputStream();
    for (size_t i = 0; i < count; ++i)
    {
      ObjectPtr solution = sampler->sample(context);
      FitnessPtr fitness = evaluate(context, solution);
      ObjectPtr perturbedSolution = perturbator->sample(context, solution);
      FitnessPtr perturbedFitness = evaluate(context, solution);
      
    }
  }

  virtual void stopSolver(ExecutionContext& context)
  {
  }

protected:
  friend class GeneratePerturbationExamplesClass;
  SamplerPtr sampler;
  PerturbatorPtr perturbator;
  size_t count;
};
*/
class TestSolver : public PopulationBasedSolver
{
public:
  TestSolver(SamplerPtr initialSampler = SamplerPtr(), size_t populationSize = 0, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler) {}

  static SolverPtr createDefault(size_t populationSize, size_t maxGenerations)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    return new TestSolver(initialSampler, populationSize, maxGenerations);
  }

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialSampler->initialize(context, problem->getDomain());
    //subsequentSampler->initialize(context, problem->getDomain());
  }

  std::pair<ExpressionPtr, ExpressionPtr> crossOver(ExecutionContext& context, const ExpressionPtr& expression1, const ExpressionPtr& expression2, ExpressionPtr& node1, ExpressionPtr& node2)
  {
    if (context.getRandomGenerator()->sampleBool(0.9))
    {
      for (size_t i = 0; i < 2; ++i)
      {
        static const size_t maxDepth = 17;
        node1 = expression1->sampleNode(context.getRandomGenerator(), 0.9);
        node2 = expression2->sampleNode(context.getRandomGenerator(), 0.9);
        ExpressionPtr newExpression1 = expression1->cloneAndSubstitute(node1, node2);
        ExpressionPtr newExpression2 = expression2->cloneAndSubstitute(node2, node1);
        if (newExpression1->getDepth() <= maxDepth && newExpression2->getDepth() <= maxDepth)
          return std::make_pair(newExpression1, newExpression2);
      }
    }
    return std::make_pair(ExpressionPtr(), ExpressionPtr()); 
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    if (iter == 0)
    {
      population = sampleAndEvaluatePopulation(context, initialSampler, populationSize);
    }
    else
    {
      juce::File examplesFile = context.getFile("iteration" + string((int)iter) + ".txt");
      if (examplesFile.existsAsFile())
        examplesFile.deleteFile();
      juce::OutputStream* ostr = examplesFile.createOutputStream();

      size_t n = population->getNumSolutions();
      std::map<ObjectPtr, FitnessPtr> fitnessByObject;
      for (size_t i = 0; i < n; ++i)
        fitnessByObject[population->getSolution(i)] = population->getFitness(i);

      SamplerPtr parentsSampler = new TournamentSampler(objectiveComparator(0), 7);
      parentsSampler->initialize(context, problem->getDomain());
      parentsSampler->learn(context, population);

      population = new SolutionVector(population->getFitnessLimits());
      population->reserve(n);

      size_t numNegatives = 0, numPositives = 0;

      for (size_t i = 0; i < n && !callback->shouldStop(); i += 2)
      {
        ExpressionPtr solution1 = parentsSampler->sample(context);
        ExpressionPtr solution2 = parentsSampler->sample(context);
        FitnessPtr fitness1 = fitnessByObject[solution1];
        FitnessPtr fitness2 = fitnessByObject[solution2];
        ExpressionPtr node1;
        ExpressionPtr node2;
        std::pair<ExpressionPtr, ExpressionPtr> pair = crossOver(context, solution1, solution2, node1, node2);
        if (pair.first && pair.second)
        {
          FitnessPtr newFitness1 = evaluate(context, pair.first);
          FitnessPtr newFitness2 = evaluate(context, pair.second);
          bool isPositive = (newFitness1->dominates(fitness1, true) && newFitness1->dominates(fitness2, true)) ||
                          (newFitness2->dominates(fitness1, true) && newFitness2->dominates(fitness2, true));
          isPositive ? ++numPositives : ++numNegatives;
          writeExample(*ostr, solution1, solution2, node1, node2, isPositive);
          population->insertSolution(pair.first, newFitness1);
          population->insertSolution(pair.second, newFitness2);
        }
        else
        {
          population->insertSolution(solution1, fitness1);
          population->insertSolution(solution2, fitness2);
        }
      }
      std::cout << numPositives << " positive examples, " << numNegatives << " negative examples." << std::endl;
      delete ostr;
    }
    return true;
  }
   
  virtual void stopSolver(ExecutionContext& context)
  {
    population = SolutionVectorPtr();
  }

  std::map<string, size_t> featuresMap;
  std::vector<string> features;

  size_t getFeatureIndex(const string& name)
  {
    std::map<string, size_t>::const_iterator it = featuresMap.find(name);
    if (it == featuresMap.end())
    {
      size_t res = featuresMap.size();
      featuresMap[name] = res;
      features.push_back(name);
      return res;
    }
    else
      return it->second;
  }

  void addFeature(const string& name, std::set<size_t>& res)
    {res.insert(getFeatureIndex(name));}

  void makeNodeFeatures(ExpressionPtr node, const string& prefix, size_t maxDepth, std::set<size_t>& res)
  {
    FunctionExpressionPtr functionNode = node.dynamicCast<FunctionExpression>();
    if (functionNode)
    {
      addFeature(prefix + "arity" + string((int)functionNode->getNumArguments()), res);
      addFeature(prefix + "function" + functionNode->getFunction()->toShortString(), res);
    }
    else
      addFeature(prefix + "terminal" + node->toShortString(), res);

    if (maxDepth > 1)
    {
      for (size_t i = 0; i < node->getNumSubNodes(); ++i)
        makeNodeFeatures(node->getSubNode(i), prefix + "_" + string((int)i) + "_", maxDepth - 1, res);
    }
  }

  void writeExample(juce::OutputStream& ostr, ExpressionPtr expression1, ExpressionPtr expression2, ExpressionPtr node1, ExpressionPtr node2, bool isPositive)
  {
    std::set<size_t> activeFeatures;
    makeNodeFeatures(expression1, "root1", 3, activeFeatures);
    makeNodeFeatures(expression2, "root2", 3, activeFeatures);
    makeNodeFeatures(node1, "node1", 3, activeFeatures);
    makeNodeFeatures(node2, "node2", 3, activeFeatures);
    //ostr << "E1: " << expression1->toShortString() << " N1: " << node1->toShortString() << " E2: " << expression2->toShortString() << " N2: " << node2->toShortString() << " => " << (isPositive ? '+' : '-') << "\n";
    ostr << (isPositive ? "+1" : "-1");
    for (std::set<size_t>::const_iterator it = activeFeatures.begin(); it != activeFeatures.end(); ++it)
      ostr << " " << string((int)*it) << ":1";
    ostr << "\n";
    /*std::cout << (isPositive ? "+1" : "-1");
    for (std::set<size_t>::const_iterator it = activeFeatures.begin(); it != activeFeatures.end(); ++it)
      std::cout << " " << features[*it];
    std::cout << std::endl;*/
  }

protected:
  friend class TestSolverClass;

  SamplerPtr initialSampler;

  SolutionVectorPtr population;
};

extern SamplerPtr tournamentSampler(SolutionComparatorPtr comparator, size_t tournamentSize);
extern DecoratorSamplerPtr perturbatorSampler(SamplerPtr inputSampler, PerturbatorPtr perturbator, double probability);
extern DecoratorSamplerPtr binaryPerturbatorSampler(SamplerPtr inputSampler, BinaryPerturbatorPtr perturbator, double probability);

class TreeGPSamplersSolver : public PopulationBasedSolver
{
public:
  TreeGPSamplersSolver(SamplerPtr initialSampler, SamplerPtr subsequentSampler, size_t populationSize = 100, size_t numGenerations = 0)
    : PopulationBasedSolver(populationSize, numGenerations), initialSampler(initialSampler), subsequentSampler(subsequentSampler) {}
  TreeGPSamplersSolver() {}

  static SolverPtr createDefault(size_t populationSize, size_t maxGenerations, size_t tournamentSize,
                                double crossOverProbability,
                                double shrinkProbability = 0.0,
                                double swapProbability = 0.0,
                                double insertProbability = 0.0,
                                double kozaProbability = 0.0)
  {
    SamplerPtr initialSampler = binaryMixtureSampler(fullExpressionSampler(2, 5), growExpressionSampler(2, 5), 0.5);
    SamplerPtr subsequentSampler = tournamentSampler(objectiveComparator(0), tournamentSize);

    if (crossOverProbability > 0.0)
      subsequentSampler = binaryPerturbatorSampler(subsequentSampler, subTreeCrossOverExpressionPerturbator(0.9), crossOverProbability);
    if (kozaProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, kozaExpressionPerturbator(growExpressionSampler()), insertProbability);
    if (shrinkProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, shrinkExpressionPerturbator(), shrinkProbability);
    if (swapProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, swapExpressionPerturbator(0.9), swapProbability);
    if (insertProbability > 0.0)
      subsequentSampler = perturbatorSampler(subsequentSampler, insertExpressionPerturbator(), insertProbability);

    return new TreeGPSamplersSolver(initialSampler, subsequentSampler, populationSize, maxGenerations);
  }

  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    initialSampler->initialize(context, problem->getDomain());
    subsequentSampler->initialize(context, problem->getDomain());
  }

  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    if (iter == 0)
    {
      population = sampleAndEvaluatePopulation(context, initialSampler, populationSize);
    }
    else
    {
      size_t n = population->getNumSolutions();
      subsequentSampler->learn(context, population);

      std::map<ObjectPtr, FitnessPtr> fitnessByObject;
      for (size_t i = 0; i < n; ++i)
        fitnessByObject[population->getSolution(i)] = population->getFitness(i);

      population = new SolutionVector(population->getFitnessLimits());
      population->reserve(n);
      for (size_t i = 0; i < n; ++i)
      {
        ObjectPtr solution = subsequentSampler->sample(context);
        std::map<ObjectPtr, FitnessPtr>::const_iterator it = fitnessByObject.find(solution);
        FitnessPtr fitness;
        if (it == fitnessByObject.end())
          fitness = evaluate(context, solution);
        else
          fitness = it->second;
        population->insertSolution(solution, fitness);
      }
    }

    if (verbosity >= verbosityDetailed)
    {
      ScalarVariableStatisticsPtr treeSize = new ScalarVariableStatistics("treeSize");
      ScalarVariableStatisticsPtr treeDepth = new ScalarVariableStatistics("treeDepth");
      for (size_t i = 0; i < population->getNumSolutions(); ++i)
      {
        ExpressionPtr expression = population->getSolution(i).staticCast<Expression>();
        treeSize->push(expression->getTreeSize());
        treeDepth->push(expression->getDepth());
      }
      context.resultCallback("treeSize", treeSize);
      context.resultCallback("treeDepth", treeDepth);
    }
    return true;
  }

  virtual void stopSolver(ExecutionContext& context)
  {
    population = SolutionVectorPtr();
  }

protected:
  friend class TreeGPSamplersSolverClass;

  SamplerPtr initialSampler;
  SamplerPtr subsequentSampler;

  SolutionVectorPtr population;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TREE_GP_SAMPLERS_H_
