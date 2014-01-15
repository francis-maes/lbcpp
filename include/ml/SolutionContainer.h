/*-----------------------------------------.---------------------------------.
| Filename: SolutionContainer.h            | Stores a set of solutions       |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_SOLUTION_CONTAINER_H_
# define ML_SOLUTION_CONTAINER_H_

# include "predeclarations.h"
# include <ml/SolutionComparator.h>

namespace lbcpp
{

class SolutionContainer : public Object
{
public:
  typedef std::pair<ObjectPtr, FitnessPtr> SolutionAndFitness;

  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness) = 0;
  virtual void insertSolutions(SolutionContainerPtr solutions) = 0;
  virtual void removeSolution(size_t index) = 0;

  virtual size_t getNumSolutions() const = 0;
  virtual ObjectPtr getSolution(size_t index) const = 0;
  virtual FitnessPtr getFitness(size_t index) const = 0;
  virtual SolutionAndFitness getSolutionAndFitness(size_t index) const = 0;
  virtual FitnessLimitsPtr getFitnessLimits() const = 0;

  size_t getNumObjectives() const;
  FitnessLimitsPtr getEmpiricalFitnessLimits() const;
};

class SolutionVector : public SolutionContainer
{
public:
  SolutionVector(FitnessLimitsPtr limits, const std::vector<SolutionAndFitness>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr());
  SolutionVector(FitnessLimitsPtr limits);
  SolutionVector() {}
  
  /*
  ** Solution accessors
  */
  bool isEmpty() const
    {return solutions.empty();}

  virtual size_t getNumSolutions() const
    {return solutions.size();}

  virtual void removeSolution(size_t index)
    {jassert(index >= 0 && index < solutions.size()); solutions.erase(index);}

  virtual ObjectPtr getSolution(size_t index) const
    {jassert(index < solutions.size()); return solutions[index].first;}

  virtual FitnessPtr getFitness(size_t index) const
    {jassert(index < solutions.size()); return solutions[index].second;}
  
  virtual SolutionAndFitness getSolutionAndFitness(size_t index) const
    {jassert(index < solutions.size()); return solutions[index];}

  void setSolution(size_t index, ObjectPtr solution, FitnessPtr fitness = FitnessPtr())
    {jassert(index < solutions.size()); solutions[index] = std::make_pair(solution, fitness);}

  void setFitness(size_t index, FitnessPtr fitness)
    {jassert(index < solutions.size() && solutions[index].first); solutions[index].second = fitness;}

  std::vector<ObjectPtr> getObjects() const;

  void reserve(size_t size)
    {solutions.reserve(size);}

  void clear()
    {solutions.clear();}

  /*
  ** Solutions insertion
  */
  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness);
  void insertSolution(const SolutionAndFitness& solutionAndFitness)
    {insertSolution(solutionAndFitness.first, solutionAndFitness.second);}

  virtual void insertSolutions(SolutionContainerPtr solutions);

  /*
  ** Solutions comparison
  */
  bool isSorted() const
    {return comparator;}

  SolutionVectorPtr sort(const SolutionComparatorPtr& comparator, std::vector<size_t>* mapping = NULL) const; // sort from the most prefered solution to the least prefered one

  int findBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionAndFitness getBestSolution(const SolutionComparatorPtr& comparator) const;
  SolutionVectorPtr selectNBests(const SolutionComparatorPtr& comparator, size_t n) const;
  
  void computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const;
  std::vector<ParetoFrontPtr> nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping = NULL) const;

  bool strictlyDominates(const FitnessPtr& fitness) const;
  ParetoFrontPtr getParetoFront() const;

  void computeCrowdingDistances(std::vector<double>& res) const;

  void duplicateSolutionsUntilReachingSize(size_t newSize);

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
  std::vector<SolutionAndFitness> solutions;
  SolutionComparatorPtr comparator;
};

class ParetoFront : public SolutionVector
{
public:
  ParetoFront(FitnessLimitsPtr limits, const std::vector<SolutionAndFitness>& solutions, SolutionComparatorPtr comparator = SolutionComparatorPtr())
    : SolutionVector(limits, solutions, comparator) {}
  ParetoFront(FitnessLimitsPtr limits) : SolutionVector(limits) {}
  ParetoFront() {}

  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness);
  virtual void insertSolutions(SolutionContainerPtr solutions);

  double computeHyperVolume(FitnessPtr referenceFitness = FitnessPtr()) const;
};

class CrowdingArchive : public ParetoFront
{
public:
  CrowdingArchive(size_t maxSize, FitnessLimitsPtr limits, const std::vector<SolutionAndFitness>& solutions) 
    : ParetoFront(limits, solutions, paretoRankAndCrowdingDistanceComparator()), maxSize(maxSize) 
  {reserve(maxSize + 1);} // we reserve maxSize+1 because we insert a solution first, then remove the worst
  
  CrowdingArchive(size_t maxSize, FitnessLimitsPtr limits) 
    : ParetoFront(limits, std::vector<SolutionAndFitness>(), paretoRankAndCrowdingDistanceComparator()), maxSize(maxSize)
  {reserve(maxSize + 1);}
  
  CrowdingArchive(size_t maxSize = 100) : maxSize(maxSize)
  {reserve(maxSize + 1);}

  virtual void insertSolution(ObjectPtr solution, FitnessPtr fitness);

protected:
  friend class CrowdingArchiveClass;

  size_t maxSize;
};

}; /* namespace lbcpp */

#endif // !ML_SOLUTION_CONTAINER_H_
