/*-----------------------------------------.---------------------------------.
| Filename: MOOCore.cpp                    | Multi Objective Optimization    |
| Author  : Francis Maes                   | Base Classes                    |
| Started : 11/09/2012 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "MOOCore.h"
#include <MOO-EALib/Hypervolume.h>
#include <algorithm>
using namespace lbcpp;

/*
** MOOFitnessLimits
*/
MOOFitnessPtr MOOFitnessLimits::getWorstPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximised(i) ? -DBL_MAX : DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].first;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

bool MOOFitnessLimits::shouldObjectiveBeMaximised(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first;}

double MOOFitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return shouldObjectiveBeMaximised(objectiveIndex) ? 1.0 : -1.0;}

struct DominanceSolutionComparator : public MOOSolutionComparator
{
  virtual int compare(const MOOSolutionPtr& solution1, const MOOSolutionPtr& solution2) const
  {
    // TODO: optimized version to avoid two calls to MOOFitness::strictlyDominates()

    //MOOFitnessLimitsPtr limits = solution1->getFitnessLimits();
    //jassert(limits == solution2->getFitnessLimits());

    if (solution1->getFitness()->strictlyDominates(solution2->getFitness()))
      return -1;
    else if (solution2->getFitness()->strictlyDominates(solution1->getFitness()))
      return 1;
    else
      return 0;
  }
};

MOOSolutionComparatorPtr MOOFitnessLimits::makeDominanceComparator() const
  {return new DominanceSolutionComparator();}

struct LexicographicSolutionComparator : public MOOSolutionComparator
{
  virtual int compare(const MOOSolutionPtr& solution1, const MOOSolutionPtr& solution2) const
  {
    MOOFitnessLimitsPtr limits = solution1->getFitnessLimits();
    jassert(limits == solution2->getFitnessLimits());

    for (size_t i = 0; i < limits->getNumObjectives(); ++i)
    {
      double value1 = solution1->getFitness()->getValue(i);
      double value2 = solution2->getFitness()->getValue(i);
      if (value1 != value2)
      {
        double deltaValue = limits->getObjectiveSign(i) * (value2 - value1);
        return deltaValue > 0 ? 1 : -1;
      }
    }
    return 0;
  }
};

MOOSolutionComparatorPtr MOOFitnessLimits::makeLexicographicComparator() const
  {return new LexicographicSolutionComparator();}

struct ObjectiveSolutionComparator : public MOOSolutionComparator
{
  ObjectiveSolutionComparator(size_t objectiveIndex, double objectiveSign)
    : objectiveIndex(objectiveIndex), objectiveSign(objectiveSign) {}

  virtual int compare(const MOOSolutionPtr& solution1, const MOOSolutionPtr& solution2) const
  {
    double value1 = solution1->getFitness()->getValue(objectiveIndex);
    double value2 = solution1->getFitness()->getValue(objectiveIndex);
    double deltaValue = objectiveSign * (value2 - value1);
    return deltaValue > 0 ? 1 : (deltaValue < 0 ? -1 : 0);
  }

private:
  size_t objectiveIndex;
  double objectiveSign;
};

MOOSolutionComparatorPtr MOOFitnessLimits::makeObjectiveComparator(size_t objectiveIndex) const
  {return new ObjectiveSolutionComparator(objectiveIndex, getObjectiveSign(objectiveIndex));}

/*
** MOOFitness
*/
MOOFitness::MOOFitness(const std::vector<double>& values, const MOOFitnessLimitsPtr& limits)
  : values(values), limits(limits)
{
}

bool MOOFitness::dominates(const MOOFitnessPtr& other, bool strictly) const
{
  jassert(other->limits == limits);

  size_t numObjectivesForWhichThisIsBetter = 0;
  for (size_t i = 0; i < values.size(); ++i)
  {
    double delta = (other->values[i] - values[i]) * limits->getObjectiveSign(i);
    if (delta > 0)
      return false; // other is better than this
    else if (delta < 0)
      ++numObjectivesForWhichThisIsBetter;
  }
  return !strictly || (numObjectivesForWhichThisIsBetter > 0);
}

bool MOOFitness::strictlyDominates(const MOOFitnessPtr& other) const
  {return dominates(other, true);}

bool MOOFitness::isBetterForAtLeastOneObjectiveThan(const MOOFitnessPtr& other, bool strictly) const
{
  jassert(other->limits == limits);
  for (size_t i = 0; i < values.size(); ++i)
  {
    double delta = (other->values[i] - values[i]) * limits->getObjectiveSign(i);
    if ((strictly && delta < 0) || (!strictly && delta <= 0))
      return true;
  }
  return false;
}

MOOFitnessPtr MOOFitness::makeWorstCombination(const MOOFitnessPtr& fitness1, const MOOFitnessPtr& fitness2)
{
  jassert(fitness1->limits == fitness2->limits);
  size_t n = fitness1->values.size();
  std::vector<double> res(n);
  for (size_t i = 0; i < n; ++i)
  {
    double objective1 = fitness1->getValue(i);
    double objective2 = fitness2->getValue(i);
    if (fitness1->limits->getObjectiveSign(i) * (objective2 - objective1) > 0)
      res[i] = objective1;
    else
      res[i] = objective2;
  }
  return new MOOFitness(res, fitness1->limits);
}

int MOOFitness::compare(const ObjectPtr& otherObject) const
{
  MOOFitnessPtr otherFitness = otherObject.dynamicCast<MOOFitness>();
  if (!otherFitness)
    return Object::compare(otherObject);
  if (limits != otherFitness->limits)
    return limits < otherFitness->limits ? -1 : 1;
  if (values != otherFitness->values)
    return values < otherFitness->values ? -1 : 1;
  return 0;
}

String MOOFitness::toShortString() const
{
  String res = "(";
  for (size_t i = 0; i < values.size(); ++i)
  {
    res += Variable(values[i]).toShortString();
    if (i < values.size() - 1)
      res += ", ";
  }
  res += ")";
  return res;
}

std::vector<double> MOOFitness::getValuesToBeMinimized() const
{
  std::vector<double> res = getValues();
  for (size_t i = 0; i < res.size(); ++i)
    if (limits->shouldObjectiveBeMaximised(i))
      res[i] = -res[i];
  return res;
}

/*
** MOOSolutionSet
*/
MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits, const std::vector<MOOSolutionPtr>& solutions, MOOSolutionComparatorPtr comparator)
  : limits(limits), solutions(solutions), comparator(comparator)
{
}

MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits)
  : limits(limits)
{
}

void MOOSolutionSet::addSolution(const MOOSolutionPtr& solution)
{
  solutions.push_back(solution);
  comparator = MOOSolutionComparatorPtr(); // mark as unsorted
}

void MOOSolutionSet::addSolution(const ObjectPtr& object, const MOOFitnessPtr& fitness)
  {addSolution(new MOOSolution(object, fitness));}

void MOOSolutionSet::addSolutions(const MOOSolutionSetPtr& otherSolutions)
{
  size_t offset = solutions.size();
  size_t n = otherSolutions->getNumSolutions();
  solutions.resize(offset + n);
  for (size_t i = 0; i < n; ++i)
    solutions[i + offset] = otherSolutions->solutions[i];
  comparator = MOOSolutionComparatorPtr(); // mark as unsorted
}

std::vector<ObjectPtr> MOOSolutionSet::getObjects() const
{
  std::vector<ObjectPtr> res(solutions.size());
  for (size_t i = 0; i < res.size(); ++i)
    res[i] = solutions[i]->getObject();
  return res;
}

struct WrapSolutionComparator
{
  WrapSolutionComparator(const MOOSolutionComparatorPtr& comparator)
    : comparator(comparator) {}

  MOOSolutionComparatorPtr comparator;

  bool operator()(const MOOSolutionPtr& a, const MOOSolutionPtr& b) const
    {return comparator->compare(a, b) < 0;}
};

MOOSolutionSetPtr MOOSolutionSet::sort(const MOOSolutionComparatorPtr& comparator) const
{
  if (this->comparator == comparator)
    return refCountedPointerFromThis(this);
  
  MOOSolutionSetPtr res = cloneAndCast<MOOSolutionSet>();
  std::sort(res->solutions.begin(), res->solutions.end(), WrapSolutionComparator(comparator));
  res->comparator = comparator; // mark as sorted
  return res;
}

int MOOSolutionSet::findBestSolution(const MOOSolutionComparatorPtr& comparator) const
{
  if (solutions.empty())
    return -1;
  if (this->comparator == comparator)
    return 0; // already sorted: the best is at first position

  MOOSolutionPtr bestSolution = solutions[0];
  size_t bestSolutionIndex = 0;
  for (size_t i = 1; i < solutions.size(); ++i)
    if (comparator->compare(solutions[i], bestSolution) == -1)
    {
      bestSolution = solutions[i];
      bestSolutionIndex = i;
    }

  return (int)bestSolutionIndex;
}

MOOSolutionPtr MOOSolutionSet::getBestSolution(const MOOSolutionComparatorPtr& comparator) const
{
  int index = findBestSolution(comparator);
  return index >= 0 ? solutions[index] : MOOSolutionPtr();
}

MOOSolutionSetPtr MOOSolutionSet::selectNBests(const MOOSolutionComparatorPtr& comparator, size_t n) const
{
  if (n >= solutions.size())
    return refCountedPointerFromThis(this);

  MOOSolutionSetPtr res = sort(comparator);
  res->solutions.resize(n);
  return res;
}

MOOFitnessLimitsPtr MOOSolutionSet::getEmpiricalLimits() const
{
  size_t n = limits->getNumDimensions();
  std::vector< std::pair<double, double> > res(n, std::make_pair(DBL_MAX, -DBL_MAX));

  for (size_t i = 0; i < solutions.size(); ++i)
  {
    MOOFitnessPtr fitness = solutions[i]->getFitness();
    for (size_t j = 0; j < n; ++j)
    {
      double value = fitness->getValue(j);
      if (value < res[j].first)
        res[j].first = value;
      if (value > res[j].second)
        res[j].second = value;
    }
  }
  return new MOOFitnessLimits(res);
}

bool MOOSolutionSet::strictlyDominates(const MOOFitnessPtr& fitness) const
{
  for (size_t i = 0; i < solutions.size(); ++i)
    if (solutions[i]->getFitness()->strictlyDominates(fitness))
      return true;
  return false;
}

MOOParetoFrontPtr MOOSolutionSet::getParetoFront() const
{
  std::vector<MOOSolutionPtr> res;
  for (size_t i = 0; i < solutions.size(); ++i)
    if (!strictlyDominates(solutions[i]->getFitness()))
      res.push_back(solutions[i]);
  return new MOOParetoFront(limits, res, comparator);
}

std::vector<MOOParetoFrontPtr> MOOSolutionSet::nonDominatedSort() const
{
  MOOSolutionComparatorPtr comparator = limits->makeDominanceComparator();

  size_t n = solutions.size();
  std::vector<size_t> dominationCounter(n, 0); // by how many others am I dominated?
  std::vector< std::vector<size_t> > dominationIndices(n); // who do I dominate?

  std::vector<MOOSolutionPtr> front;
  std::vector<size_t> currentFrontIndices;
  for (size_t i = 0; i < n; ++i)
  {
    for (size_t j = 0; j < n; ++j)
    {
      int c = comparator->compare(solutions[i], solutions[j]);
      if (c == -1)
        dominationIndices[i].push_back(j);
      else if (c == 1)
        dominationCounter[i]++;
    }
    if (dominationCounter[i] == 0)
    {
      front.push_back(solutions[i]);
      currentFrontIndices.push_back(i);
    }
  }

  std::vector<MOOParetoFrontPtr> res;
  if (currentFrontIndices.empty())
    return res;
  
  res.push_back(new MOOParetoFront(limits, front, this->comparator));
  while (true)
  {
    std::vector<size_t> nextFrontIndices;
    front.clear();
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
          front.push_back(solutions[index]);
          nextFrontIndices.push_back(index);
        }
      }
    }
    currentFrontIndices.swap(nextFrontIndices);

    jassert(front.size() == currentFrontIndices.size());
    if (front.size())
      res.push_back(new MOOParetoFront(limits, front, this->comparator));
    else
      break;
  }

#ifdef JUCE_DEBUG
  size_t debugSize = 0;
  for (size_t i = 0; i < res.size(); ++i)
    debugSize += res[i]->getNumSolutions();
  jassert(debugSize == solutions.size());
#endif // JUCE_DEBUG
  return res;
}

void MOOSolutionSet::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const MOOSolutionSetPtr& target = t.staticCast<MOOSolutionSet>();
  target->limits = limits;
  target->solutions = solutions;
  target->comparator = comparator;
}

#if 0
MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits, const Map& elements)
  : limits(limits), m(elements)
{
  size = 0;
  for (Map::const_iterator it = elements.begin(); it != elements.end(); ++it)
    size += it->second.size();
}

MOOSolutionSet::MOOSolutionSet(MOOFitnessLimitsPtr limits)
  : limits(limits), size(0)
{
}

MOOSolutionSet::MOOSolutionSet() : size(0)
{
}

void MOOSolutionSet::add(const MOOSolutionSetPtr& solutions)
{
  jassert(solutions);
  for (Map::const_iterator it = solutions->m.begin(); it != solutions->m.end(); ++it)
  {
    Map::iterator it2 = m.find(it->first);
    std::vector<ObjectPtr>& target = (it2 == m.end() ? m[it->first] : it2->second);
    target.reserve(target.size() + it->second.size());
    for (size_t i = 0; i < it->second.size(); ++i)
      target.push_back(it->second[i]);
  }
  size += solutions->size;
}

void MOOSolutionSet::add(const ObjectPtr& solution, const MOOFitnessPtr& fitness)
{
  jassert(fitness);
  Map::iterator it = m.find(fitness);
  if (it == m.end())
    m[fitness] = std::vector<ObjectPtr>(1, solution);
  else
    it->second.push_back(solution);
  ++size;
}

void MOOSolutionSet::getSolutions(std::vector<ObjectPtr>& res) const
{
  res.resize(size);
  size_t index = 0;
  for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    MOOFitnessPtr fitness = it->first;
    const std::vector<ObjectPtr>& solutions = it->second;
    for (size_t i = 0; i < solutions.size(); ++i)
      {jassert(index < size); res[index++] = solutions[i];}
  }
  jassert(index == size);
}

void MOOSolutionSet::getFitnesses(std::vector<MOOFitnessPtr>& res) const
{
  res.resize(m.size());
  size_t index = 0;
  for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
    res[index++] = it->first;
}

void MOOSolutionSet::getSolutionAndFitnesses(std::vector< std::pair<MOOFitnessPtr, ObjectPtr> >& res) const
{
  res.reserve(size);
  for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
  {
    MOOFitnessPtr fitness = it->first;
    const std::vector<ObjectPtr>& solutions = it->second;
    for (size_t i = 0; i < solutions.size(); ++i)
      res.push_back(std::make_pair(fitness, solutions[i]));
  }
}

void MOOSolutionSet::getSolutionsByFitness(const MOOFitnessPtr& fitness, std::vector<ObjectPtr>& res) const
{
  Map::const_iterator it = m.find(fitness);
  if (it == m.end())
    res.clear();
  else
    res = it->second;
}

struct CompareIthObjective
{
  CompareIthObjective(const std::vector<MOOFitnessPtr>& fitnesses, size_t i, bool isMaximisation)
    : fitnesses(fitnesses), i(i), isMaximisation(isMaximisation) {}

  const std::vector<MOOFitnessPtr>& fitnesses;
  size_t i;
  bool isMaximisation;

  bool operator()(size_t a, size_t b)
    {return isMaximisation ? fitnesses[a]->getValue(i) > fitnesses[b]->getValue(i) : fitnesses[a]->getValue(i) < fitnesses[b]->getValue(i);}
};

void MOOSolutionSet::computeCrowdingDistances(std::vector<double>& res) const
{
  std::vector<MOOFitnessPtr> fitnesses;
  getFitnesses(fitnesses);
  size_t n = fitnesses.size();

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
    std::sort(order.begin(), order.end(), CompareIthObjective(fitnesses, i, limits->shouldObjectiveBeMaximised(i)));
    double bestValue = fitnesses[order.front()]->getValue(i);
    double worstValue = fitnesses[order.back()]->getValue(i);
    double invRange = 1.0 / (worstValue - bestValue);
    res[order.front()] = DBL_MAX;
    res[order.back()] = DBL_MAX;
    for (size_t j = 1; j < order.size() - 1; ++j)
      res[order[j]] += (fitnesses[order[j+1]]->getValue(i) - fitnesses[order[j-1]]->getValue(i)) * invRange;
  }
}


#endif // 0

/*
** MOOParetoFront
*/
void MOOParetoFront::addSolutionAndUpdateFront(const ObjectPtr& object, const MOOFitnessPtr& fitness)
{
  std::vector<MOOSolutionPtr> newSolutions;
  newSolutions.reserve(solutions.size());
  for (size_t i = 0; i < solutions.size(); ++i)
  {
    MOOFitnessPtr solutionFitness = solutions[i]->getFitness();
    if (solutionFitness->strictlyDominates(fitness))
      return; // dominated
    if (!fitness->strictlyDominates(solutionFitness))
      newSolutions.push_back(solutions[i]);
  }
  newSolutions.push_back(new MOOSolution(object, fitness));
  solutions.swap(newSolutions);
}

double MOOParetoFront::computeHyperVolume(const MOOFitnessPtr& referenceFitness) const
{
  if (isEmpty())
    return 0.0;

  size_t numObjectives = limits->getNumObjectives();
  if (numObjectives == 1)
  {
    double bestValue = getBestSolution(limits->makeObjectiveComparator(0))->getFitness()->getValue(0);
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
      MOOFitnessPtr fitness = solutions[i]->getFitness();
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

/*
** MOOOptimizer
*/
MOOParetoFrontPtr MOOOptimizer::optimize(ExecutionContext& context, MOOProblemPtr problem)
{
  this->front = new MOOParetoFront(problem->getFitnessLimits());
  this->problem = problem;

  optimize(context);
  MOOParetoFrontPtr res = front;

  this->problem = MOOProblemPtr();
  this->front = MOOParetoFrontPtr();

  return res;
}

MOOFitnessPtr MOOOptimizer::evaluate(ExecutionContext& context, const ObjectPtr& object)
{
  jassert(problem && front);
  MOOFitnessPtr fitness = problem->evaluate(context, object);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  front->addSolutionAndUpdateFront(object, fitness);
  return fitness;
}

MOOFitnessPtr MOOOptimizer::evaluateAndSave(ExecutionContext& context, const ObjectPtr& object, MOOSolutionSetPtr solutions)
{
  MOOFitnessPtr fitness = evaluate(context, object);
  solutions->addSolution(object, fitness);
  return fitness;
}

ObjectPtr MOOOptimizer::sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler)
  {return problem->getObjectDomain()->projectIntoDomain(sampler->sample(context));}
 
MOOFitnessPtr MOOOptimizer::sampleAndEvaluateSolution(ExecutionContext& context, MOOSamplerPtr sampler, MOOSolutionSetPtr population)
{
  ObjectPtr solution = sampleSolution(context, sampler); 
  return population ? evaluateAndSave(context, solution, population) : evaluate(context, solution);
}

MOOSolutionSetPtr MOOOptimizer::sampleAndEvaluatePopulation(ExecutionContext& context, MOOSamplerPtr sampler, size_t populationSize)
{
  MOOSolutionSetPtr res = new MOOSolutionSet(problem->getFitnessLimits());
  for (size_t i = 0; i < populationSize; ++i)
    sampleAndEvaluateSolution(context, sampler, res);
  jassert(res->getNumSolutions() == populationSize);
  return res;
}

void MOOOptimizer::learnSampler(ExecutionContext& context, MOOSolutionSetPtr solutions, MOOSamplerPtr sampler)
  {sampler->learn(context, solutions->getObjects());}
