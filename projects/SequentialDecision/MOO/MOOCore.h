/*-----------------------------------------.---------------------------------.
| Filename: MOOCore.h                      | Multi Objective Optimization    |
| Author  : Francis Maes                   | Base Classes                    |
| Started : 11/09/2012 18:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_CORE_H_
# define LBCPP_MOO_CORE_H_

# include <lbcpp/Core/Object.h>

namespace lbcpp
{

class MOODomain;
typedef ReferenceCountedObjectPtr<MOODomain> MOODomainPtr;

class ContinuousMOODomain;
typedef ReferenceCountedObjectPtr<ContinuousMOODomain> ContinuousMOODomainPtr;

class MOOFitness;
typedef ReferenceCountedObjectPtr<MOOFitness> MOOFitnessPtr;

class MOOFitnessLimits;
typedef ReferenceCountedObjectPtr<MOOFitnessLimits> MOOFitnessLimitsPtr;

class MOOProblem;
typedef ReferenceCountedObjectPtr<MOOProblem> MOOProblemPtr;

class MOOSolutionSet;
typedef ReferenceCountedObjectPtr<MOOSolutionSet> MOOSolutionSetPtr;

class MOOParetoFront;
typedef ReferenceCountedObjectPtr<MOOParetoFront> MOOParetoFrontPtr;

class MOOOptimizer;
typedef ReferenceCountedObjectPtr<MOOOptimizer> MOOOptimizerPtr;

class MOOSampler;
typedef ReferenceCountedObjectPtr<MOOSampler> MOOSamplerPtr;

class MOODomain : public Object
{
public:
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

  bool shouldObjectiveBeMaximized(size_t objectiveIndex) const;
  double getObjectiveSign(size_t objectiveIndex) const; // 1 for maximisation and -1 for minimisation

  MOOFitnessPtr getWorstPossibleFitness(bool useInfiniteValues = false) const;
};

class MOOFitness : public Object
{
public:
  MOOFitness(const std::vector<double>& values, const MOOFitnessLimitsPtr& limits);
  MOOFitness() {}

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

class MOOSolutionSet : public Object
{
public:
  MOOSolutionSet(MOOFitnessLimitsPtr limits);
  MOOSolutionSet();

  void add(const ObjectPtr& solution, const MOOFitnessPtr& fitness);
  void add(const MOOSolutionSetPtr& solutions);
  void getSolutions(std::vector<ObjectPtr>& res) const;
  void getSolutionAndFitnesses(std::vector< std::pair<MOOFitnessPtr, ObjectPtr> >& res) const;
  void getSolutionsByFitness(const MOOFitnessPtr& fitness, std::vector<ObjectPtr>& res) const;

  const MOOFitnessLimitsPtr& getTheoreticalLimits() const
    {return limits;}

  MOOFitnessLimitsPtr getEmpiricalLimits() const;

  size_t getNumObjectives() const
    {return limits->getNumObjectives();}

  size_t getNumElements() const
    {return size;}

  typedef std::map<MOOFitnessPtr, std::vector<ObjectPtr>, ObjectComparator > ParetoMap;

  const ParetoMap& getMap() const
    {return m;}

protected:
  friend class MOOSolutionSetClass;

  MOOFitnessLimitsPtr limits;
  ParetoMap m;
  size_t size;
};

class MOOParetoFront : public MOOSolutionSet
{
public:
  MOOParetoFront(MOOFitnessLimitsPtr limits) : MOOSolutionSet(limits) {}
  MOOParetoFront() {}

  void insert(const ObjectPtr& solution, const MOOFitnessPtr& fitness);

  double computeHyperVolume(const MOOFitnessPtr& referenceFitness) const;
};

class MOOProblem : public Object
{
public:
  virtual MOODomainPtr getSolutionDomain() const = 0;
  virtual MOOFitnessLimitsPtr getFitnessLimits() const = 0;

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution) = 0;

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

  MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution);
  MOOFitnessPtr evaluateAndSave(ExecutionContext& context, const ObjectPtr& solution, MOOSolutionSetPtr archive);
  MOOFitnessPtr sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr population = MOOSolutionSetPtr());
  MOOSolutionSetPtr sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize);
  void learnSampler(ExecutionContext& context, MOOSolutionSetPtr population, MOOSamplerPtr sampler);
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
