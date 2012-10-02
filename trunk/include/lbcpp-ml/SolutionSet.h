/*-----------------------------------------.---------------------------------.
| Filename: SolutionSet.h                  | Stores a set of solutions       |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLUTION_SET_H_
# define LBCPP_ML_SOLUTION_SET_H_

# include "Solution.h"

namespace lbcpp
{

class SolutionSet : public Object
{
public:
  SolutionSet(FitnessLimitsPtr limits, const std::vector<SolutionPtr>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr());
  SolutionSet(FitnessLimitsPtr limits);
  SolutionSet() {}
  
  /*
  ** Solution accessors
  */
  bool isEmpty() const
    {return solutions.empty();}

  size_t getNumSolutions() const
    {return solutions.size();}

  SolutionPtr getSolution(size_t index) const
    {jassert(index < solutions.size()); return solutions[index];}

  FitnessPtr getFitness(size_t index) const
    {return getSolution(index)->getFitness();}

  std::vector<ObjectPtr> getObjects() const;

  /*
  ** Solutions insertion
  */
  void addSolution(const SolutionPtr& solution);
  void addSolution(const ObjectPtr& object, const FitnessPtr& fitness);
  void addSolutions(const SolutionSetPtr& solutions);

  /*
  ** Solutions comparison
  */
  bool isSorted() const
    {return comparator;}

  SolutionSetPtr sort(const SolutionComparatorPtr& comparator, std::vector<size_t>* mapping = NULL) const; // sort from the most prefered solution to the least prefered one

  int findBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionPtr getBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionSetPtr selectNBests(const SolutionComparatorPtr& comparator, size_t n) const;
  
  void computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const;
  std::vector<ParetoFrontPtr> nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping = NULL) const;

  bool strictlyDominates(const FitnessPtr& fitness) const;
  ParetoFrontPtr getParetoFront() const;

  void computeCrowdingDistances(std::vector<double>& res) const;

  /*
  ** Fitness limits
  */
  const FitnessLimitsPtr& getFitnessLimits() const
    {return limits;}

  FitnessLimitsPtr getEmpiricalLimits() const;

  size_t getNumObjectives() const
    {return limits->getNumObjectives();}

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class SolutionSetClass;

  FitnessLimitsPtr limits;
  std::vector<SolutionPtr> solutions;
  SolutionComparatorPtr comparator;
};

class ParetoFront : public SolutionSet
{
public:
  ParetoFront(FitnessLimitsPtr limits, const std::vector<SolutionPtr>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : SolutionSet(limits, solutions, comparator) {}
  ParetoFront(FitnessLimitsPtr limits) : SolutionSet(limits) {}
  ParetoFront() {}

  void addSolutionAndUpdateFront(const ObjectPtr& object, const FitnessPtr& fitness);
  double computeHyperVolume(const FitnessPtr& referenceFitness) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLUTION_SET_H_
