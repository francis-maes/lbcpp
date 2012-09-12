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

  MOOFitnessPtr getWorstPossibleFitness() const;
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

class MOOParetoFront : public Object
{
public:
  MOOParetoFront(MOOFitnessLimitsPtr limits);
  MOOParetoFront();

  void insert(const ObjectPtr& solution, const MOOFitnessPtr& fitness);
  void getSolutions(std::vector< std::pair<MOOFitnessPtr, ObjectPtr> >& res) const;
  void getSolutionsByFitness(const MOOFitnessPtr& fitness, std::vector<ObjectPtr>& res) const;

  const MOOFitnessLimitsPtr& getTheoreticalLimits() const
    {return limits;}

  MOOFitnessLimitsPtr getEmpiricalLimits() const;

  size_t getNumObjectives() const
    {return limits->getNumObjectives();}

  size_t getNumSolutions() const
    {return size;}

  double computeHyperVolume(const MOOFitnessPtr& referenceFitness) const;

  typedef std::map<MOOFitnessPtr, std::vector<ObjectPtr>, ObjectComparator > ParetoMap;

  const ParetoMap& getParetoMap() const
    {return m;}

protected:
  friend class MOOParetoFrontClass;

  MOOFitnessLimitsPtr limits;
  ParetoMap m;
  size_t size;
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
  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoFront) = 0;

protected:
  typedef std::pair<ObjectPtr, MOOFitnessPtr> SolutionAndFitnessPair;
};

class MOOSampler : public Object
{
public:
  virtual ObjectPtr sample(ExecutionContext& context, const MOODomainPtr& domain) const = 0;
  virtual void reinforce(const ObjectPtr& solution) = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_CORE_H_
