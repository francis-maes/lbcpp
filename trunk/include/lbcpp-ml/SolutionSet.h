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

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLUTION_SET_H_
