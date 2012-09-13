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
using namespace lbcpp;

/*
** MOOFitnessLimits
*/
MOOFitnessPtr MOOFitnessLimits::getWorstPossibleFitness(bool useInfiniteValues) const
{
  std::vector<double> res(limits.size());
  if (useInfiniteValues)
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = shouldObjectiveBeMaximized(i) ? -DBL_MAX : DBL_MAX;
  else
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = limits[i].first;
  return MOOFitnessPtr(new MOOFitness(res, refCountedPointerFromThis(this)));
}

bool MOOFitnessLimits::shouldObjectiveBeMaximized(size_t objectiveIndex) const
  {return limits[objectiveIndex].second > limits[objectiveIndex].first;}

double MOOFitnessLimits::getObjectiveSign(size_t objectiveIndex) const
  {return shouldObjectiveBeMaximized(objectiveIndex) ? 1.0 : -1.0;}

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
    if (limits->shouldObjectiveBeMaximized(i))
      res[i] = -res[i];
  return res;
}

/*
** MOOSolutionSet
*/
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

MOOFitnessLimitsPtr MOOSolutionSet::getEmpiricalLimits() const
{
  size_t n = limits->getNumDimensions();
  std::vector< std::pair<double, double> > res(n, std::make_pair(DBL_MAX, -DBL_MAX));

  for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
    for (size_t i = 0; i < n; ++i)
    {
      double value = it->first->getValue(i);
      if (value < res[i].first)
        res[i].first = value;
      if (value > res[i].second)
        res[i].second = value;
    }

  return new MOOFitnessLimits(res);
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
    std::sort(order.begin(), order.end(), CompareIthObjective(fitnesses, i, limits->shouldObjectiveBeMaximized(i)));
    double bestValue = fitnesses[order.front()]->getValue(i);
    double worstValue = fitnesses[order.back()]->getValue(i);
    double invRange = 1.0 / (worstValue - bestValue);
    res[order.front()] = DBL_MAX;
    res[order.back()] = DBL_MAX;
    for (size_t j = 1; j < order.size() - 1; ++j)
      res[order[j]] += (fitnesses[order[j+1]]->getValue(i) - fitnesses[order[j-1]]->getValue(i)) * invRange;
  }
}

std::vector<MOOParetoFrontPtr> MOOSolutionSet::nonDominatedSort() const
{
  std::vector<MOOFitnessPtr> fitnesses;
  getFitnesses(fitnesses);
  size_t n = fitnesses.size();

  std::vector<size_t> dominationCounter(n, 0); // by how many others am I dominated?
  std::vector< std::vector<size_t> > dominationIndices(n); // who do I dominate?

  Map front;
  std::vector<size_t> currentFrontIndices;
  for (size_t i = 0; i < n; ++i)
  {
    for (size_t j = 0; j < n; ++j)
      if (fitnesses[i]->strictlyDominates(fitnesses[j]))
        dominationIndices[i].push_back(j);
      else if (fitnesses[j]->strictlyDominates(fitnesses[i]))
        dominationCounter[i]++;
    if (dominationCounter[i] == 0)
    {
      front[fitnesses[i]] = m.find(fitnesses[i])->second;
      currentFrontIndices.push_back(i);
    }
  }

  std::vector<MOOParetoFrontPtr> res;
  if (currentFrontIndices.empty())
    return res;
  
  res.push_back(new MOOParetoFront(limits, front));
  while (true)
  {
    Map nextFront;
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
          MOOFitnessPtr fitness = fitnesses[index];
          nextFrontIndices.push_back(index);
          front[fitness] = m.find(fitness)->second;
        }
      }
    }
    currentFrontIndices.swap(nextFrontIndices);

    jassert(front.size() == currentFrontIndices.size());
    if (front.size())
      res.push_back(new MOOParetoFront(limits, front));
    else
      break;
  }

#ifdef JUCE_DEBUG
  size_t debugSize = 0;
  for (size_t i = 0; i < res.size(); ++i)
    debugSize += res[i]->getNumElements();
  jassert(debugSize == size);
#endif // JUCE_DEBUG
  return res;
}

/*
** MOOParetoFront
*/
void MOOParetoFront::insert(const ObjectPtr& solution, const MOOFitnessPtr& fitness)
{
  for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
    if (it->first->strictlyDominates(fitness))
      return; // dominated

  Map::iterator it = m.find(fitness);
  if (it == m.end())
  {
    Map::iterator nxt;
    for (it = m.begin(); it != m.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (fitness->strictlyDominates(it->first))
      {
        size -= it->second.size();
        m.erase(it);
      }
    }
    m[fitness] = std::vector<ObjectPtr>(1, solution);
    ++size;
  }
  else
  {
    it->second.push_back(solution);
    ++size;
  }
}

double MOOParetoFront::computeHyperVolume(const MOOFitnessPtr& referenceFitness) const
{
  if (m.empty())
    return 0.0;

  size_t numObjectives = limits->getNumObjectives();
  if (numObjectives == 1)
  {
    double bestValue = (limits->shouldObjectiveBeMaximized(0) ? m.rbegin()->first->getValue(0) : m.begin()->first->getValue(0));
    double res = limits->getObjectiveSign(0) * (bestValue - referenceFitness->getValue(0));
    return res > 0.0 ? res : 0.0;
  }
  else
  {
    // remove points that fall outside the hypervolume and convert to minimization problem
    std::vector<double> points(m.size() * numObjectives);
    size_t numPoints = 0;
    for (Map::const_iterator it = m.begin(); it != m.end(); ++it)
      if (!referenceFitness->isBetterForAtLeastOneObjectiveThan(it->first))
      {
        std::vector<double> val = it->first->getValuesToBeMinimized();
        memcpy(&points[numPoints * numObjectives], &val[0], sizeof (double) * numObjectives);
        ++numPoints;
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

MOOFitnessPtr MOOOptimizer::evaluate(ExecutionContext& context, const ObjectPtr& solution)
{
  jassert(problem && front);
  MOOFitnessPtr fitness = problem->evaluate(context, solution);
  for (size_t i = 0; i < fitness->getNumValues(); ++i)
    jassert(isNumberValid(fitness->getValue(i)));
  front->insert(solution, fitness);
  return fitness;
}

MOOFitnessPtr MOOOptimizer::evaluateAndSave(ExecutionContext& context, const ObjectPtr& solution, MOOSolutionSetPtr archive)
{
  MOOFitnessPtr fitness = evaluate(context, solution);
  archive->add(solution, fitness);
  return fitness;
}

ObjectPtr MOOOptimizer::sampleSolution(ExecutionContext& context, MOOSamplerPtr sampler)
  {return problem->getSolutionDomain()->projectIntoDomain(sampler->sample(context));}
 
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
  jassert(res->getNumElements() == populationSize);
  return res;
}

void MOOOptimizer::learnSampler(ExecutionContext& context, MOOSolutionSetPtr population, MOOSamplerPtr sampler)
{
  std::vector<ObjectPtr> samples;
  population->getSolutions(samples);
  sampler->learn(context, samples);
}