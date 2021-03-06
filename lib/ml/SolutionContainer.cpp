/*-----------------------------------------.---------------------------------.
| Filename: SolutionContainer.cpp          | Containers of solutions         |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/SolutionContainer.h>
#include <ml/SolutionComparator.h>
#include <MOO-EALib/Hypervolume.h>
#include <algorithm>
#include <iostream>
#include <fstream>
using namespace lbcpp;

/*
** SolutionContainer
*/
size_t SolutionContainer::getNumObjectives() const
  {return getFitnessLimits()->getNumObjectives();}

FitnessLimitsPtr SolutionContainer::getEmpiricalFitnessLimits() const
{
  FitnessLimitsPtr limits = getFitnessLimits();
  size_t n = limits->getNumDimensions();
  std::vector< std::pair<double, double> > res(n, std::make_pair(DBL_MAX, -DBL_MAX));

  size_t numSolutions = getNumSolutions();
  for (size_t i = 0; i < numSolutions; ++i)
  {
    FitnessPtr fitness = getFitness(i);
    for (size_t j = 0; j < n; ++j)
    {
      double value = fitness->getValue(j);
      if (value < res[j].first)
        res[j].first = value;
      if (value > res[j].second)
        res[j].second = value;
    }
  }
  return new FitnessLimits(res);
}

void SolutionContainer::insertSolutions(SolutionContainerPtr solutions)
{
  size_t n = solutions->getNumSolutions();
  for (size_t i = 0; i < n; ++i)
    insertSolution(solutions->getSolution(i), solutions->getFitness(i));
}

/*
** SolutionVector
*/
SolutionVector::SolutionVector(FitnessLimitsPtr limits, const std::vector<SolutionAndFitness>& solutions, SolutionComparatorPtr comparator)
  : limits(limits), solutions(solutions), comparator(comparator)
{
}

SolutionVector::SolutionVector(FitnessLimitsPtr limits)
  : limits(limits)
{
}

void SolutionVector::insertSolution(ObjectPtr solution, FitnessPtr fitness)
{
  solutions.push_back(SolutionAndFitness(solution, fitness));
  comparator = SolutionComparatorPtr(); // mark as unsorted
}

void SolutionVector::insertSolutions(SolutionContainerPtr otherSolutions)
{
  jassert(otherSolutions);
  size_t offset = solutions.size();
  size_t n = otherSolutions->getNumSolutions();
  solutions.resize(offset + n);
  SolutionVectorPtr other = otherSolutions.dynamicCast<SolutionVector>();
  if (other)
  {
    for (size_t i = 0; i < n; ++i)
      solutions[i + offset] = other->solutions[i];
  }
  else
  {
    for (size_t i = 0; i < n; ++i)
      solutions[i + offset] = SolutionAndFitness(otherSolutions->getSolution(i), otherSolutions->getFitness(i));
  }
  comparator = SolutionComparatorPtr(); // mark as unsorted
}

std::vector<ObjectPtr> SolutionVector::getObjects() const
{
  std::vector<ObjectPtr> res(solutions.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = solutions[i].first;
  return res;
}

struct WrapSolutionComparator
{
  WrapSolutionComparator(const SolutionComparatorPtr& comparator)
    : comparator(comparator) {}

  SolutionComparatorPtr comparator;

  bool operator()(size_t a, size_t b) const
    {return comparator->compareSolutions(a, b) < 0;}
};

SolutionVectorPtr SolutionVector::sort(const SolutionComparatorPtr& comparator, std::vector<size_t>* mapping) const
{
  if (this->comparator == comparator)
    return refCountedPointerFromThis(this);
  size_t n = solutions.size();
  
  // compute ordered indices
  std::vector<size_t> localMapping;
  if (!mapping)
    mapping = &localMapping;
  mapping->resize(n);
  for (size_t i = 0; i < n; ++i)
    (*mapping)[i] = i;
  comparator->initialize(refCountedPointerFromThis(this));
  std::sort(mapping->begin(), mapping->end(), WrapSolutionComparator(comparator));

  // copy and re-order
  SolutionVectorPtr res = new SolutionVector(limits);
  res->solutions.resize(n);
  for (size_t i = 0; i < n; ++i)
    res->solutions[i] = solutions[(*mapping)[i]];
  res->comparator = comparator; // mark as sorted
  return res;
}

int SolutionVector::findBestSolution(const SolutionComparatorPtr& comparator) const
{
  if (solutions.empty())
    return -1;
  if (this->comparator == comparator)
    return 0; // already sorted: the best is at first position

  comparator->initialize(refCountedPointerFromThis(this));
  size_t bestSolutionIndex = 0;
  for (size_t i = 1; i < solutions.size(); ++i)
    if (comparator->compareSolutions(i, bestSolutionIndex) == -1)
      bestSolutionIndex = i;

  return (int)bestSolutionIndex;
}

SolutionContainer::SolutionAndFitness SolutionVector::getBestSolution(const SolutionComparatorPtr& comparator) const
{
  int index = findBestSolution(comparator);
  return index >= 0 ? solutions[index] : SolutionAndFitness();
}

void SolutionVector::duplicateSolutionsUntilReachingSize(size_t newSize)
{
  jassert(newSize >= solutions.size());
  size_t oldSize = solutions.size();
  size_t i = 0;
  solutions.resize(newSize);
  for (size_t s = oldSize; s < newSize; ++s)
  {
    solutions[s] = solutions[i];
    i = (i + 1) % oldSize;
  }
}

SolutionVectorPtr SolutionVector::selectNBests(const SolutionComparatorPtr& comparator, size_t n) const
{
  if (n >= solutions.size())
    return refCountedPointerFromThis(this);

  SolutionVectorPtr res = sort(comparator);
  res->solutions.resize(n);
  return res;
}

bool SolutionVector::strictlyDominates(const FitnessPtr& fitness) const
{
  for (size_t i = 0; i < solutions.size(); ++i)
    if (solutions[i].second->strictlyDominates(fitness))
      return true;
  return false;
}

ParetoFrontPtr SolutionVector::getParetoFront() const
{
  std::vector<SolutionAndFitness> res;
  for (size_t i = 0; i < solutions.size(); ++i)
    if (!strictlyDominates(solutions[i].second))
      res.push_back(solutions[i]);
  return new ParetoFront(limits, res, comparator);
}

// mapping: solutionPosition -> frontIndex, positionInFront
void SolutionVector::computeParetoRanks(std::vector< std::pair<size_t, size_t> >& mapping, std::vector<size_t>& countPerRank) const
{
  size_t n = solutions.size();
  if (!n)
    return;

  std::vector<size_t> dominationCounter(n, 0); // by how many others am I dominated?
  std::vector< std::vector<size_t> > dominationIndices(n); // who do I dominate?

  mapping.resize(n);

  std::vector<size_t> currentFrontIndices;
  SolutionComparatorPtr dom = dominanceComparator();
  dom->initialize(refCountedPointerFromThis(this));
  for (size_t i = 0; i < n; ++i)
  {
    for (size_t j = 0; j < n; ++j)
    {
      int c = dom->compareSolutions(i, j);
      if (c == -1)
        dominationIndices[i].push_back(j);
      else if (c == 1)
        dominationCounter[i]++;
    }
    if (dominationCounter[i] == 0)
    {
      mapping[i] = std::make_pair(0, currentFrontIndices.size());
      currentFrontIndices.push_back(i);
    }
  }

  jassert(currentFrontIndices.size() > 0);
  countPerRank.push_back(currentFrontIndices.size());
  
  for (size_t currentRank = 1; true; ++currentRank)
  {
    std::vector<size_t> nextFrontIndices;
    for (size_t i = 0; i < currentFrontIndices.size(); ++i)
    {
      const std::vector<size_t>& indices = dominationIndices[currentFrontIndices[i]];
      for (size_t j = 0; j < indices.size(); ++j)
      {
        size_t index = indices[j];
        size_t& counter = dominationCounter[index];
        jassert(counter > 0);
        --counter;
        if (counter == 0)
        {
          mapping[index] = std::make_pair(currentRank, nextFrontIndices.size());
          nextFrontIndices.push_back(index);
        }
      }
    }
    currentFrontIndices.swap(nextFrontIndices);

    if (currentFrontIndices.size())
      countPerRank.push_back(currentFrontIndices.size());
    else
      break;
  }

#ifdef JUCE_DEBUG
  size_t totalSize = 0;
  for (size_t i = 0; i < countPerRank.size(); ++i)
    totalSize += countPerRank[i];
  jassert(totalSize == n);
#endif // JUCE_DEBUG
}

std::vector<ParetoFrontPtr> SolutionVector::nonDominatedSort(std::vector< std::pair<size_t, size_t> >* mapping) const
{
  std::vector< std::pair<size_t, size_t> > localMapping;
  if (!mapping)
    mapping = &localMapping;
  std::vector<size_t> countPerRank;
  computeParetoRanks(*mapping, countPerRank);

  std::vector<ParetoFrontPtr> res(countPerRank.size());
  for (size_t i = 0; i < res.size(); ++i)
  {
    ParetoFrontPtr front = new ParetoFront(limits);
    front->solutions.reserve(countPerRank[i]);
    front->comparator = comparator;
    res[i] = front;
  }

  for (size_t i = 0; i < mapping->size(); ++i)
    res[(*mapping)[i].first]->solutions.push_back(solutions[i]);
  return res;
}

void SolutionVector::computeCrowdingDistances(std::vector<double>& res) const
{
  size_t n = solutions.size();

  res.resize(n, 0.0);
  if (n <= 2)
  {
    for (size_t i = 0; i < n; ++i)
      res[i] = DBL_MAX;
    return;
  }
  
  std::vector<size_t> order(n);
  for (size_t i = 0; i < n; ++i)
    order[i] = i;

  for (size_t i = 0; i < limits->getNumObjectives(); ++i)
  {
    std::vector<size_t> mapping;
    SolutionVectorPtr ordered = sort(objectiveComparator(i), &mapping);
    double bestValue = ordered->getFitness(0)->getValue(i);
    double worstValue = ordered->getFitness(n - 1)->getValue(i);
    double invRange = 1.0 / (worstValue - bestValue);
    res[mapping[0]] = DBL_MAX;
    res[mapping[n-1]] = DBL_MAX;
    for (size_t j = 1; j < n - 1; ++j)
      res[mapping[j]] += (ordered->getFitness(j+1)->getValue(i) - ordered->getFitness(j-1)->getValue(i)) * invRange;
  }
}

void SolutionVector::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const SolutionVectorPtr& target = t.staticCast<SolutionVector>();
  target->limits = limits;
  target->solutions = solutions;
  target->comparator = comparator;
}

/*
** ParetoFront
*/

double ParetoFront::computeSpreadIndicator() const
{
  size_t n = getNumSolutions();
  if (n < 3)
    return 0.0;
  double dmean = 0.0;
  for (size_t i = 1; i < n; ++i)
    dmean += getFitness(i-1)->euclideanDistanceTo(getFitness(i));
  dmean /= n;
  double df = getFitness(0)->euclideanDistanceTo(getFitness(1));
  double dl = getFitness(n-2)->euclideanDistanceTo(getFitness(n-1));
  double sum = 0.0;
  for (size_t i = 2; i < n-1; ++i)
    sum += fabs(getFitness(i-1)->euclideanDistanceTo(getFitness(i))-dmean);
  return ((df + dl + sum)/(df + dl + (n-1)*dmean));
}

ParetoFront::ParetoFront(FitnessLimitsPtr limits, const string& path) : SolutionVector(limits)
{
  size_t n = limits->getNumObjectives();
  size_t i = 0;
  std::fstream stream(path.toUTF8(), std::ios_base::in);
  double value;
  std::vector<double> v(n);
  while (stream >> value)
  {
    v[i++] = value;
    if (i % n == 0)
    {
      insertSolution(ObjectPtr(), new Fitness(v, limits));
      i = 0;
    }
  }
}

void ParetoFront::insertSolution(ObjectPtr solution, FitnessPtr fitness)
{
  std::vector<SolutionAndFitness> newSolutions;
  newSolutions.reserve(solutions.size());
  std::vector<SolutionAndFitness>::iterator insertPos = newSolutions.begin();
  bool found = false;
  for (size_t i = 0; i < solutions.size(); ++i)
  {
    FitnessPtr solutionFitness = solutions[i].second;
    if (solutionFitness->strictlyDominates(fitness))
      return; // dominated
    if (solution.exists() && solutions[i].first->compare(solution) == 0 ||  // already in the front
        solutions[i].second->compare(fitness) == 0)  // already a solution that has the same fitness  (this test may become flagable in the future)
      return;
    if (!fitness->strictlyDominates(solutionFitness))
    {
      if (!found && fitness->getValues() <= solutionFitness->getValues())
      {
        found = true;
        newSolutions.push_back(SolutionAndFitness(solution, fitness));
      }
      newSolutions.push_back(solutions[i]);
    }
  }
  if (!found)
    newSolutions.push_back(SolutionAndFitness(solution, fitness));
  solutions.swap(newSolutions);
}

void ParetoFront::insertSolutions(SolutionContainerPtr solutions)
{
  SolutionContainer::insertSolutions(solutions);
}

double ParetoFront::computeHyperVolume(FitnessPtr referenceFitness) const
{
  if (isEmpty())
    return 0.0;

  if (!referenceFitness)
    referenceFitness = limits->getWorstPossibleFitness();

  size_t numObjectives = limits->getNumObjectives();
  if (numObjectives == 1)
  {
    double bestValue = getBestSolution(objectiveComparator(0)).second->getValue(0);
    double res = limits->getObjectiveSign(0) * (bestValue - referenceFitness->getValue(0));
    return res > 0.0 ? res : 0.0;
  }
  else
  {
    // remove points that fall outside the hypervolume and convert to minimization problem
    std::vector<double> points(solutions.size() * numObjectives);
    size_t numPoints = 0;
    for (size_t i = 0; i < solutions.size(); ++i)
    {
      FitnessPtr fitness = solutions[i].second;
      if (!referenceFitness->isBetterForAtLeastOneObjectiveThan(fitness))
      {
        std::vector<double> val = fitness->getValuesToBeMinimized();
        memcpy(&points[numPoints * numObjectives], &val[0], sizeof (double) * numObjectives);
        ++numPoints;
      }
    }

    if (!numPoints)
      return 0.0;

    std::vector<double> ref = referenceFitness->getValuesToBeMinimized();

    // shark implementation
    return hypervolume(&points[0], &ref[0], (unsigned int)numObjectives, (unsigned int)numPoints);
  }
}

double ParetoFront::computeMultiplicativeEpsilonIndicator(ParetoFrontPtr referenceFront) const
{
  if (isEmpty() || getNumObjectives() != referenceFront->getNumObjectives())
  {
    jassertfalse;
    return 0.0;
  }

  double result = 1.0;
  for (size_t i = 0; i < getNumSolutions(); ++i)
  {
    FitnessPtr fitness = getFitness(i);
    double eps = DBL_MAX;
    for (size_t j = 0; j < referenceFront->getNumSolutions(); ++j)
      eps = std::min(eps, fitness->multiplicativeEpsilon(referenceFront->getFitness(j)));
    result = std::max(result, eps);
  }
  return result;
}

double ParetoFront::computeAdditiveEpsilonIndicator(ParetoFrontPtr referenceFront) const
{
  if (isEmpty() || getNumObjectives() != referenceFront->getNumObjectives())
  {
    jassertfalse;
    return 0.0;
  }

  double result = 0.0;
  for (size_t i = 0; i < getNumSolutions(); ++i)
  {
    FitnessPtr fitness = getFitness(i);
    double eps = DBL_MAX;
    for (size_t j = 0; j < referenceFront->getNumSolutions(); ++j)
      eps = std::min(eps, fitness->additiveEpsilon(referenceFront->getFitness(j)));
    result = std::max(result, eps);
  }
  return result;
}

void CrowdingArchive::insertSolution(ObjectPtr solution, FitnessPtr fitness)
{
  ParetoFront::insertSolution(solution, fitness);
  if (getNumSolutions() > maxSize)
  {
    /* now we remove the worst solution */
    int worst = 0;
    double worstDistance = DBL_MAX;
    for (size_t i = 1; i < getNumSolutions()-1; ++i)
    {
      double d = 0.0;
      for (size_t j = 0; j < fitness->getNumValues(); ++j)
        d += fabs((getFitness(i+1)->getValue(j) - getFitness(i-1)->getValue(j)) / (limits->getUpperLimit(j) - limits->getLowerLimit(j)));
      if (d < worstDistance)
      {
        worst = i;
        worstDistance = d;
      }
    }
    solutions.erase(solutions.begin() + worst);
  }
}
