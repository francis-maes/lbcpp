/*-----------------------------------------.---------------------------------.
| Filename: MOOCore.h                      | Multi Objective Optimization    |
| Author  : Francis Maes                   | Base Classes                    |
| Started : 11/09/2012 18:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_CORE_H_
# define LBCPP_MOO_CORE_H_

# include "predeclarations.h"

namespace lbcpp
{

class MOODomain : public Object
{
public:
  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const
    {return object;}
};

class ContinuousMOODomain : public MOODomain
{
public:
  ContinuousMOODomain(const std::vector< std::pair<double, double> >& limits)
    : limits(limits) {}
  ContinuousMOODomain() {}

  size_t getNumDimensions() const
    {return limits.size();}

  double getLowerLimit(size_t dimension) const
    {jassert(dimension < limits.size()); return limits[dimension].first;}
  
  double getUpperLimit(size_t dimension) const
    {jassert(dimension < limits.size()); return limits[dimension].second;}

  const std::vector< std::pair<double, double> >& getLimits() const
    {return limits;}

  DenseDoubleVectorPtr sampleUniformly(RandomGeneratorPtr random) const;
  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class ContinuousMOODomainClass;

  std::vector< std::pair<double, double> > limits;
};

class MOOFitnessLimits : public ContinuousMOODomain
{
public:
  MOOFitnessLimits(const std::vector< std::pair<double, double> >& limits)
    : ContinuousMOODomain(limits) {}
  MOOFitnessLimits() {}

  size_t getNumObjectives() const
    {return limits.size();}

  void addObjective(double worstValue, double bestValue)
    {limits.push_back(std::make_pair(worstValue, bestValue));}

  bool shouldObjectiveBeMaximised(size_t objectiveIndex) const;
  double getObjectiveSign(size_t objectiveIndex) const; // 1 for maximisation and -1 for minimisation

  MOOFitnessPtr getWorstPossibleFitness(bool useInfiniteValues = false) const;
  MOOFitnessPtr getBestPossibleFitness(bool useInfiniteValues = false) const;

  double computeUtopicHyperVolume() const;
};

class MOOFitness : public Object
{
public:
  MOOFitness(const std::vector<double>& values, const MOOFitnessLimitsPtr& limits);
  MOOFitness() {}

  const MOOFitnessLimitsPtr& getLimits() const
    {return limits;}

  size_t getNumValues() const
    {return values.size();}

  double getValue(size_t i) const
    {jassert(i < values.size()); return values[i];}

  const std::vector<double>& getValues() const
    {return values;}

  std::vector<double> getValuesToBeMinimized() const;

  bool dominates(const MOOFitnessPtr& other, bool strictly = false) const;
  bool strictlyDominates(const MOOFitnessPtr& other) const;
  bool isBetterForAtLeastOneObjectiveThan(const MOOFitnessPtr& other, bool strictly = true) const;
  
  virtual String toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;

  static MOOFitnessPtr makeWorstCombination(const MOOFitnessPtr& fitness1, const MOOFitnessPtr& fitness2);

protected:
  friend class MOOFitnessClass;

  std::vector<double> values;
  MOOFitnessLimitsPtr limits;
};

class MOOSolution : public Object
{
public:
  MOOSolution(const ObjectPtr& object, const MOOFitnessPtr& fitness)
    : object(object), fitness(fitness) {}
  MOOSolution() {}

  /*
  ** Object
  */
  const ObjectPtr& getObject() const
    {return object;}

  /*
  ** Fitness
  */
  const MOOFitnessPtr& getFitness() const
    {return fitness;}

  const MOOFitnessLimitsPtr& getFitnessLimits() const
    {return fitness->getLimits();}

protected:
  friend class MOOSolutionClass;

  ObjectPtr object;
  MOOFitnessPtr fitness;
};

class MOOSolutionComparator : public Object
{
public:
  virtual void initialize(const MOOSolutionSetPtr& solutions) = 0;

  // returns -1 if solution1 is prefered, +1 if solution2 is prefered and 0 if there is no preference between the two solutions
  virtual int compare(size_t index1, size_t index2) = 0;
};

extern MOOSolutionComparatorPtr objectiveComparator(size_t index);
extern MOOSolutionComparatorPtr lexicographicComparator();
extern MOOSolutionComparatorPtr dominanceComparator();
extern MOOSolutionComparatorPtr paretoRankAndCrowdingDistanceComparator();

class MOOSolutionSet : public Object
{
public:
  MOOSolutionSet(MOOFitnessLimitsPtr limits, const std::vector<MOOSolutionPtr>& solutions, MOOSolutionComparatorPtr comparator = MOOSolutionComparatorPtr());
  MOOSolutionSet(MOOFitnessLimitsPtr limits);
  MOOSolutionSet() {}
  
  /*
  ** Solution accessors
  */
  bool isEmpty() const
    {return solutions.empty();}

  size_t getNumSolutions() const
    {return solutions.size();}

  MOOSolutionPtr getSolution(size_t index) const
    {jassert(index < solutions.size()); return solutions[index];}

  MOOFitnessPtr getFitness(size_t index) const
    {return getSolution(index)->getFitness();}

  std::vector<ObjectPtr> getObjects() const;

  /*
  ** Solutions insertion
  */
  void addSolution(const MOOSolutionPtr& solution);
  void addSolution(const ObjectPtr& object, const MOOFitnessPtr& fitness);
  void addSolutions(const MOOSolutionSetPtr& solutions);

  /*
  ** Solutions comparison
  */
  bool isSorted() const
    {return comparator;}

  MOOSolutionSetPtr sort(const MOOSolutionComparatorPtr& comparator, std::vector<size_t>* mapping = NULL) const; // sort from the most prefered solution to the least prefered one

  int findBestSolution(const MOOSolutionComparatorPtr& comparator) const;
  MOOSolutionPtr getBestSolution(const MOOSolutionComparatorPtr& comparator) const;
  MOOSolutionSetPtr selectNBests(const MOOSolutionComparatorPtr& comparator, size_t n) const;
  
  void computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const;
  std::vector<MOOParetoFrontPtr> nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping = NULL) const;

  bool strictlyDominates(const MOOFitnessPtr& fitness) const;
  MOOParetoFrontPtr getParetoFront() const;

  void computeCrowdingDistances(std::vector<double>& res) const;

  /*
  ** Fitness limits
  */
  const MOOFitnessLimitsPtr& getFitnessLimits() const
    {return limits;}

  MOOFitnessLimitsPtr getEmpiricalLimits() const;

  size_t getNumObjectives() const
    {return limits->getNumObjectives();}

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class MOOSolutionSetClass;

  MOOFitnessLimitsPtr limits;
  std::vector<MOOSolutionPtr> solutions;
  MOOSolutionComparatorPtr comparator;
};

class MOOParetoFront : public MOOSolutionSet
{
public:
  MOOParetoFront(MOOFitnessLimitsPtr limits, const std::vector<MOOSolutionPtr>& solutions, MOOSolutionComparatorPtr comparator = MOOSolutionComparatorPtr())
    : MOOSolutionSet(limits, solutions, comparator) {}
  MOOParetoFront(MOOFitnessLimitsPtr limits) : MOOSolutionSet(limits) {}
  MOOParetoFront() {}

  void addSolutionAndUpdateFront(const ObjectPtr& object, const MOOFitnessPtr& fitness);
  double computeHyperVolume(const MOOFitnessPtr& referenceFitness) const;
};

class MOOProblem : public Object
{
public:
  virtual MOODomainPtr getObjectDomain() const = 0;
  virtual MOOFitnessLimitsPtr getFitnessLimits() const = 0;

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object) = 0;

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {jassertfalse; return ObjectPtr();}

  virtual bool shouldStop() const
    {return false;}

  size_t getNumObjectives() const
    {return getFitnessLimits()->getNumObjectives();}
};

class MOOOptimizer : public Object
{
public:
  MOOOptimizer() : verbosity(verbosityQuiet) {}
  
  enum Verbosity
  {
    verbosityQuiet = 0,
    verbosityProgressAndResult,
    verbosityDetailed,
    verbosityAll
  };

  MOOParetoFrontPtr optimize(ExecutionContext& context, MOOProblemPtr problem, Verbosity verbosity = verbosityQuiet);

  virtual void configure(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr front, Verbosity verbosity = verbosityQuiet);
  virtual void optimize(ExecutionContext& context) = 0;
  virtual void clear(ExecutionContext& context);
  
  MOOProblemPtr getProblem() const
    {return problem;}

  MOOParetoFrontPtr getFront() const
    {return front;}

  double computeHyperVolume() const
    {return front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness());}

protected:
  typedef std::pair<ObjectPtr, MOOFitnessPtr> SolutionAndFitnessPair;

  MOOProblemPtr problem;
  MOOParetoFrontPtr front;
  Verbosity verbosity;

  MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);
  MOOFitnessPtr evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, MOOSolutionSetPtr solutions);
  ObjectPtr sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler);
  MOOFitnessPtr sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr solutions = MOOSolutionSetPtr());
  MOOSolutionSetPtr sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize);
  void learnSampler(ExecutionContext& context, MOOSolutionSetPtr solutions, MOOSamplerPtr sampler);
};

class IterativeOptimizer : public MOOOptimizer
{
public:
  IterativeOptimizer(size_t numIterations = 0)
    : numIterations(numIterations) {}

  virtual bool iteration(ExecutionContext& context, size_t iter) = 0; // returns false if the optimizer has converged

  virtual void optimize(ExecutionContext& context);

protected:
  friend class IterativeOptimizerClass;

  size_t numIterations;
};

class PopulationBasedMOOOptimizer : public IterativeOptimizer
{
public:
  PopulationBasedMOOOptimizer(size_t populationSize = 100, size_t numGenerations = 0)
    : IterativeOptimizer(numGenerations), populationSize(populationSize) {}

protected:
  friend class PopulationBasedMOOOptimizerClass;

  size_t populationSize;
};

class MOOSampler : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const MOODomainPtr& domain) = 0;

  virtual ObjectPtr sample(ExecutionContext& context) const = 0;
  virtual bool isDegenerate() const // returns true if the sampler has became deterministic
    {return false;}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects) = 0;
  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object) = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_CORE_H_
