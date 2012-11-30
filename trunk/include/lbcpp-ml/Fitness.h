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

class FitnessLimits : public ScalarVectorDomain
{
public:
  FitnessLimits(const std::vector< std::pair<double, double> >& limits)
    : ScalarVectorDomain(limits) {}
  FitnessLimits() {}

  size_t getNumObjectives() const
    {return limits.size();}

  void addObjective(double worstValue, double bestValue)
    {limits.push_back(std::make_pair(worstValue, bestValue));}

  bool shouldObjectiveBeMaximised(size_t objectiveIndex) const;
  double getObjectiveSign(size_t objectiveIndex) const; // 1 for maximisation and -1 for minimisation

  FitnessPtr getWorstPossibleFitness(bool useInfiniteValues = false) const;
  FitnessPtr getBestPossibleFitness(bool useInfiniteValues = false) const;

  double computeUtopicHyperVolume() const;
};

class Fitness : public Object
{
public:
  Fitness(const std::vector<double>& values, const FitnessLimitsPtr& limits);
  Fitness(double value, const FitnessLimitsPtr& limits);
  Fitness() {}

  const FitnessLimitsPtr& getLimits() const
    {return limits;}

  size_t getNumValues() const
    {return values.size();}

  double getValue(size_t i) const
    {jassert(i < values.size()); return values[i];}

  const std::vector<double>& getValues() const
    {return values;}

  std::vector<double> getValuesToBeMinimized() const;

  bool dominates(const FitnessPtr& other, bool strictly = false) const;
  bool strictlyDominates(const FitnessPtr& other) const;
  bool isBetterForAtLeastOneObjectiveThan(const FitnessPtr& other, bool strictly = true) const;
  
  virtual string toShortString() const;
  virtual int compare(const ObjectPtr& otherObject) const;

  static FitnessPtr makeWorstCombination(const FitnessPtr& fitness1, const FitnessPtr& fitness2);

  virtual double toDouble() const;
  
protected:
  friend class FitnessClass;

  std::vector<double> values;
  FitnessLimitsPtr limits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FITNESS_H_
