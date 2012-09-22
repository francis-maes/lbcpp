/*-----------------------------------------.---------------------------------.
| Filename: Fitness.h                      | Fitness                         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FITNESS_H_
# define LBCPP_ML_FITNESS_H_

# include "Domain.h"

namespace lbcpp
{

class MOOFitnessLimits : public ContinuousDomain
{
public:
  MOOFitnessLimits(const std::vector< std::pair<double, double> >& limits)
    : ContinuousDomain(limits) {}
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

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FITNESS_H_
