/*-----------------------------------------.---------------------------------.
| Filename: SolutionContainer.h            | Stores a set of solutions       |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SOLUTION_CONTAINER_H_
# define LBCPP_ML_SOLUTION_CONTAINER_H_

# include "Solution.h"

namespace lbcpp
{

class SolutionContainer : public Object
{
public:
  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness) = 0;
  virtual void insertSolutions(SolutionContainerPtr solutions) = 0;

  virtual size_t getNumSolutions() const = 0;
  virtual ObjectPtr getSolution(size_t index) const = 0;
  virtual FitnessPtr getFitness(size_t index) const = 0;
  virtual FitnessLimitsPtr getFitnessLimits() const = 0;

  size_t getNumObjectives() const;
  FitnessLimitsPtr getEmpiricalFitnessLimits() const;
};

class SolutionVector : public SolutionContainer
{
public:
  SolutionVector(FitnessLimitsPtr limits, const std::vector<SolutionPtr>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr());
  SolutionVector(FitnessLimitsPtr limits);
  SolutionVector() {}
  
  /*
  ** Solution accessors
  */
  bool isEmpty() const
    {return solutions.empty();}

  size_t getNumSolutions() const
    {return solutions.size();}

  ObjectPtr getSolution(size_t index) const
    {jassert(index < solutions.size()); return solutions[index]->getObject();}

  FitnessPtr getFitness(size_t index) const
    {jassert(index < solutions.size()); return solutions[index]->getFitness();}

  std::vector<ObjectPtr> getObjects() const;

  /*
  ** Solutions insertion
  */
  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness);
  virtual void insertSolutions(SolutionContainerPtr solutions);

  /*
  ** Solutions comparison
  */
  bool isSorted() const
    {return comparator;}

  SolutionVectorPtr sort(const SolutionComparatorPtr& comparator, std::vector<size_t>* mapping = NULL) const; // sort from the most prefered solution to the least prefered one

  int findBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionPtr getBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionVectorPtr selectNBests(const SolutionComparatorPtr& comparator, size_t n) const;
  
  void computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const;
  std::vector<ParetoFrontPtr> nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping = NULL) const;

  bool strictlyDominates(const FitnessPtr& fitness) const;
  ParetoFrontPtr getParetoFront() const;

  void computeCrowdingDistances(std::vector<double>& res) const;

  /*
  ** Fitness limits
  */
  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  /*
  ** Object
  */
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class SolutionVectorClass;

  FitnessLimitsPtr limits;
  std::vector<SolutionPtr> solutions;
  SolutionComparatorPtr comparator;
};

class ParetoFront : public SolutionVector
{
public:
  ParetoFront(FitnessLimitsPtr limits, const std::vector<SolutionPtr>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : SolutionVector(limits, solutions, comparator) {}
  ParetoFront(FitnessLimitsPtr limits) : SolutionVector(limits) {}
  ParetoFront() {}

  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness);

  double computeHyperVolume(FitnessPtr referenceFitness = FitnessPtr()) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SOLUTION_CONTAINER_H_
