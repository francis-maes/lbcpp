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

  DenseDoubleVectorPtr sampleUniformly(RandomGeneratorPtr random) const
  {
    size_t n = limits.size();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n, 0.0));
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, random->sampleDouble(getLowerLimit(i), getUpperLimit(i)));
    return res;
  }

  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr solution = object.staticCast<DenseDoubleVector>();
    DenseDoubleVectorPtr res;
    size_t n = limits.size();
    for (size_t i = 0; i < n; ++i)
    {
      double value = solution->getValue(i);
      double projectedValue = juce::jlimit(limits[i].first, limits[i].second, value);
      if (value != projectedValue)
      {
        if (!res)
          res = solution->cloneAndCast<DenseDoubleVector>(); // allocate in a lazy way
        res->setValue(i, projectedValue);
      }
    }
    return res ? res : object;
  }

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

  MOOSolutionComparatorPtr makeDominanceComparator() const;
  MOOSolutionComparatorPtr makeLexicographicComparator() const;
  MOOSolutionComparatorPtr makeObjectiveComparator(size_t objectiveIndex) const;
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

  const ObjectPtr& getObject() const
    {return object;}

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
  // returns -1 if solution1 is prefered, +1 if solution2 is prefered and 0 if there is no preference between the two solutions
  virtual int compare(const MOOSolutionPtr& solution1, const MOOSolutionPtr& solution2) const = 0;
};

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

  MOOSolutionSetPtr sort(const MOOSolutionComparatorPtr& comparator) const; // sort from the most prefered solution to the least prefered one

  int findBestSolution(const MOOSolutionComparatorPtr& comparator) const;
  MOOSolutionPtr getBestSolution(const MOOSolutionComparatorPtr& comparator) const;
  MOOSolutionSetPtr selectNBests(const MOOSolutionComparatorPtr& comparator, size_t n) const;
  
  std::vector<MOOParetoFrontPtr> nonDominatedSort() const;
  bool strictlyDominates(const MOOFitnessPtr& fitness) const;
  MOOParetoFrontPtr getParetoFront() const;

  /*
  ** Fitness limits
  */
  const MOOFitnessLimitsPtr& getTheoreticalLimits() const
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

#if 0
class MOOSolutionSet : public Object
{
public:
  typedef std::map<MOOFitnessPtr, std::vector<ObjectPtr>, ObjectComparator > Map;

  MOOSolutionSet(MOOFitnessLimitsPtr limits, const Map& elements);
  MOOSolutionSet(MOOFitnessLimitsPtr limits);
  MOOSolutionSet();

  void add(const ObjectPtr& solution, const MOOFitnessPtr& fitness);
  void add(const MOOSolutionSetPtr& solutions);
  void getSolutions(std::vector<ObjectPtr>& res) const;
  void getFitnesses(std::vector<MOOFitnessPtr>& res) const;
  void getSolutionAndFitnesses(std::vector< std::pair<MOOFitnessPtr, ObjectPtr> >& res) const;
  void getSolutionsByFitness(const MOOFitnessPtr& fitness, std::vector<ObjectPtr>& res) const;
  void computeCrowdingDistances(std::vector<double>& res) const;

  const MOOFitnessLimitsPtr& getTheoreticalLimits() const
    {return limits;}

  MOOFitnessLimitsPtr getEmpiricalLimits() const;

  size_t getNumObjectives() const
    {return limits->getNumObjectives();}

  size_t getNumElements() const
    {return size;}

  std::vector<MOOParetoFrontPtr> nonDominatedSort() const;

  const Map& getMap() const
    {return m;}

protected:
  friend class MOOSolutionSetClass;

  MOOFitnessLimitsPtr limits;
  Map m;
  size_t size;
};
#endif // 0

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
  MOOParetoFrontPtr optimize(ExecutionContext& context, MOOProblemPtr problem);

  virtual void optimize(ExecutionContext& context) = 0;

protected:
  typedef std::pair<ObjectPtr, MOOFitnessPtr> SolutionAndFitnessPair;

  MOOProblemPtr problem;
  MOOParetoFrontPtr front;

  MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);
  MOOFitnessPtr evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, MOOSolutionSetPtr solutions);
  ObjectPtr sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler);
  MOOFitnessPtr sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr solutions = MOOSolutionSetPtr());
  MOOSolutionSetPtr sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize);
  void learnSampler(ExecutionContext& context, MOOSolutionSetPtr solutions, MOOSamplerPtr sampler);
};

class PopulationBasedMOOOptimizer : public MOOOptimizer
{
public:
  PopulationBasedMOOOptimizer(size_t populationSize = 100, size_t numGenerations = 0)
    : populationSize(populationSize), numGenerations(numGenerations) {}

protected:
  friend class PopulationBasedMOOOptimizerClass;

  size_t populationSize;
  size_t numGenerations;
};

class MOOSampler : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const MOODomainPtr& domain) = 0;

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& solutions) = 0;
  virtual void reinforce(ExecutionContext& context, const ObjectPtr& solution) = 0;

  virtual ObjectPtr sample(ExecutionContext& context) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_CORE_H_
