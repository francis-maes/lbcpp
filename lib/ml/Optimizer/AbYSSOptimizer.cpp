/*-----------------------------------------.---------------------------------.
| Filename: AbYSSOptimizer.cpp             | AbYSS Optimizer                 |
| Author  : Denny Verbeeck                 |                                 |
| Started : 08/01/2014 22:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "AbYSSOptimizer.h"
#include <ml/Domain.h>
#include <ml/SolutionComparator.h>

namespace lbcpp
{

void AbYSSOptimizer::init(ExecutionContext& context)
{
  solutionSet = new SolutionVector(problem->getFitnessLimits());
  solutionSet->reserve(populationSize);

  subSetSize_ = populationSize * 1000;
  
  archive = new SolutionVector(problem->getFitnessLimits());
  archive->reserve(archiveSize_);
  
  refSet1 = new SolutionVector(problem->getFitnessLimits());
  refSet1->reserve(refSet1Size_);
  
  refSet2 = new SolutionVector(problem->getFitnessLimits());
  refSet2->reserve(refSet2Size_);
  
  subSet = new SolutionVector(problem->getFitnessLimits());
  subSet->reserve(subSetSize_);

  distancesToSolutionSet = std::map<ObjectPtr, double>();
  marked = std::map<ObjectPtr, bool>();
  
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  size_t numDimensions = domain->getNumDimensions();
  
  crossoverOperator = sbxCrossover(0.9, 20.0);
  improvementOperator = localSearchMutation(polynomialMutation(1.0/numDimensions, 20.0), 1);
  
  numberOfSubranges_ = 4;
  sumOfFrequencyValues_ = new int[numDimensions]();
  sumOfReverseFrequencyValues_ = new int[numDimensions]();
  frequency_ = new int*[numberOfSubranges_]();
  reverseFrequency_ = new int*[numberOfSubranges_]();
  for (size_t i = 0; i < numberOfSubranges_; ++i)
  {
    frequency_[i] = new int[numDimensions]();
    reverseFrequency_[i] = new int[numDimensions]();
  }
}

void AbYSSOptimizer::cleanUp()
{
  delete[] sumOfFrequencyValues_;
  delete[] sumOfReverseFrequencyValues_;
  for (size_t i = 0; i < numberOfSubranges_; ++i)
  {
    delete[] frequency_[i];
    delete[] reverseFrequency_[i];
  }
  delete[] frequency_;
  delete[] reverseFrequency_;
}

bool AbYSSOptimizer::iterateSolver(ExecutionContext& context, size_t iter)
{
  DenseDoubleVectorPtr object;
  FitnessPtr fitness;
  if (iter == 0)
  {
    /* Create initial population */
    for (size_t i = 0; i < populationSize; ++i)
    {
      object = diversificationGeneration(context);
      improvementOperator->execute(context, problem, object);
      fitness = evaluate(context, object);
      solutionSet->insertSolution(object, fitness);
    }
  }
  else 
  {
    size_t newSolutions = 0;
    referenceSetUpdate(context, true);
    newSolutions = subSetGeneration(context);
    while (newSolutions > 0)  // New solutions are created
    {
      referenceSetUpdate(context, false);
      newSolutions = subSetGeneration(context);
    }

    // RE-START
    solutionSet->clear();
    for (size_t i = 0; i < refSet1->getNumSolutions(); ++i)
    {
      object = refSet1->getSolution(i).staticCast<DenseDoubleVector>();
      marked[refSet1->getSolution(i)] = false;
      improvementOperator->execute(context, problem, object);
      fitness = evaluate(context, object);
      solutionSet->insertSolution(object, fitness);
    }

    refSet1->clear();
    refSet2->clear();

    size_t insert = populationSize / 2;
    if (insert > archive->getNumSolutions())
      insert = archive->getNumSolutions();

    if (insert > populationSize - solutionSet->getNumSolutions())
      insert = populationSize - solutionSet->getNumSolutions();

    // Insert solutions
    for (size_t i = 0; i < insert; ++i)
    {
      object = cloneVector(context, archive->getSolution(i).staticCast<DenseDoubleVector>());
      marked[object.staticCast<Object>()] = false;
      fitness = archive->getFitness(i);
      solutionSet->insertSolution(object, fitness);
    }

    // Create the rest of solutions randomly
    while (solutionSet->getNumSolutions() < populationSize)
    {
      object = diversificationGeneration(context);
      improvementOperator->execute(context, problem, object);
      fitness = evaluate(context, object);
      marked[object.staticCast<Object>()] = false;
      solutionSet->insertSolution(object, fitness);
    }
  }
  return true;
}

DenseDoubleVectorPtr AbYSSOptimizer::diversificationGeneration(ExecutionContext& context)
{
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  size_t numDimensions = domain->getNumDimensions();
  DenseDoubleVectorPtr solution = new DenseDoubleVector(numDimensions, 0.0);
  double value = 0.0;
  int range = 0;

  for (size_t i = 0; i < numDimensions; ++i)
  {
      sumOfReverseFrequencyValues_[i] = 0 ;
      for (size_t j = 0; j < numberOfSubranges_; ++j)
      {
        reverseFrequency_[j][i] = sumOfFrequencyValues_[i] - frequency_[j][i] ;
        sumOfReverseFrequencyValues_[i] += reverseFrequency_[j][i] ;
      }

      if (sumOfReverseFrequencyValues_[i] == 0)
      {
        range = context.getRandomGenerator()->sampleInt(numberOfSubranges_);
      } 
      else 
      { 
        value = context.getRandomGenerator()->sampleInt(sumOfReverseFrequencyValues_[i]);
        range = 0;
        while (value > reverseFrequency_[range][i])
        {
          value -= reverseFrequency_[range][i];
          range++;
        } // while
      } // else            
            
      frequency_[range][i]++;
      sumOfFrequencyValues_[i]++;

      double lowerLimit = problem->getDomain().staticCast<ScalarVectorDomain>()->getLowerLimit(i);
      double upperLimit = problem->getDomain().staticCast<ScalarVectorDomain>()->getUpperLimit(i);

      double low = lowerLimit + range*(upperLimit - lowerLimit) / numberOfSubranges_;
      double high = low + (upperLimit - lowerLimit) / numberOfSubranges_;
      value = context.getRandomGenerator()->sampleDouble(low, high);
      solution->setValue(i, value);
    } // for       
    return solution ;
}

void AbYSSOptimizer::referenceSetUpdate(ExecutionContext& context, bool build)
{
  if (build)
  {
    ObjectPtr individual;
    DenseDoubleVectorPtr individualVector;
    FitnessPtr fitness;
    // Build a new reference set
    solutionSet = solutionSet->sort(spea2Comparator());
    for (size_t i = 0; i < refSet1Size_; ++i)
    {
      individual = solutionSet->getSolution(0);
      fitness = solutionSet->getFitness(0);
      solutionSet->removeSolution(0);
      marked[individual] = false;
      refSet1->insertSolution(individual, fitness);
    }
    
    // Compute Euclidean distances in solutionSet to obtain q
    // individuals, where q is refSet2Size_
    distancesToSolutionSet.clear();
    for (size_t i = 0; i < solutionSet->getNumSolutions(); ++i)
      distancesToSolutionSet[solutionSet->getSolution(i)] = distanceToSolutionSet(solutionSet->getSolution(i).staticCast<DenseDoubleVector>(), refSet1);
    
    size_t size = refSet2Size_;
    if (solutionSet->getNumSolutions() < size)
      size = solutionSet->getNumSolutions();
    
    // Build refSet2 with these q individuals
    for (size_t i = 0; i < size; ++i)
    {
      // Find the maximum minimum distance to population
      double maxMinimum = 0.0;
      size_t index = 0;
      double dist = 0.0;
      for (size_t j = 0; j < solutionSet->getNumSolutions(); ++j)
      {
        dist = distancesToSolutionSet[solutionSet->getSolution(j)];
        if (dist > maxMinimum)
        {
          maxMinimum = dist;
          index = j;
        }
      }
      individual = solutionSet->getSolution(index);
      fitness = solutionSet->getFitness(index);
      individualVector = individual.staticCast<DenseDoubleVector>();
      solutionSet->removeSolution(index);
      
      // Update distances to refSet in population
      for (size_t j = 0; j < solutionSet->getNumSolutions(); ++j)
      {
        dist = individualVector->distanceTo(solutionSet->getSolution(j).staticCast<DenseDoubleVector>());
        if (dist < maxMinimum)
          distancesToSolutionSet[solutionSet->getSolution(j)] = dist;
      }
      
      // Insert individual in refSet2
      refSet2->insertSolution(individual, fitness);
      
      // Update distances in refSet2
      for (size_t j = 0; j < refSet2->getNumSolutions(); ++j)
      {
        for (size_t k = 0; k < refSet2->getNumSolutions(); ++k)
        {
          if (j != k)
          {
            dist = refSet2->getSolution(j).staticCast<DenseDoubleVector>()->distanceTo(refSet2->getSolution(k).staticCast<DenseDoubleVector>());
            if (distancesToSolutionSet.find(refSet2->getSolution(j)) == distancesToSolutionSet.end() || dist < distancesToSolutionSet[refSet2->getSolution(j)])
              distancesToSolutionSet[refSet2->getSolution(j)] = dist;
          }
        }
      }
    }
  }
  else
  {
    // Update the reference set from the subset generation result
    ObjectPtr individual;
    DenseDoubleVectorPtr individualVector;
    FitnessPtr fitness;
    double dist = 0.0;
    for (size_t i = 0; i < subSet->getNumSolutions(); ++i)
    {
      individual = cloneVector(context, subSet->getSolution(i));
      individualVector = individual.staticCast<DenseDoubleVector>();
      improvementOperator->execute(context, problem, individualVector);
      fitness = evaluate(context, individual);

      if (refSet1Test(individual, fitness))
      {
        // update distance of refSet2
        for (size_t indSet2 = 0; indSet2 < refSet2->getNumSolutions(); ++indSet2)
        {
          dist = individualVector->distanceTo(refSet2->getSolution(indSet2).staticCast<DenseDoubleVector>());
          if (distancesToSolutionSet.find(refSet2->getSolution(indSet2)) == distancesToSolutionSet.end() || dist < distancesToSolutionSet[refSet2->getSolution(indSet2)])
            distancesToSolutionSet[refSet2->getSolution(indSet2)] = dist;
        }
      }
      else
      {
        refSet2Test(individual, fitness);
      }
    }
    subSet->clear();
  }

}

bool AbYSSOptimizer::refSet1Test(const ObjectPtr &solution, const FitnessPtr& fitness)
{
  bool dominated = false;
  size_t i = 0;
  while (i < refSet1->getNumSolutions())
  {
    if (fitness->strictlyDominates(refSet1->getFitness(i)))
      refSet1->removeSolution(i);
    else if (refSet1->getFitness(i)->strictlyDominates(fitness))
    {
      dominated = true;
      ++i;
    }
    else
    {
      if (fitness->getValues() == refSet1->getFitness(i)->getValues())
        return true;
      ++i;
    }
  }
  
  if (!dominated)
  {
    marked[solution] = false;
    if (refSet1->getNumSolutions() < refSet1Size_) // refSet1 is not full
      refSet1->insertSolution(solution, fitness);
    else
      archive->insertSolution(solution, fitness);
  }
  else
    return false;
  return true;
}
  
bool AbYSSOptimizer::refSet2Test(const ObjectPtr &solution, const FitnessPtr& fitness)
{
  DenseDoubleVectorPtr solutionVector = solution.staticCast<DenseDoubleVector>();
  double aux = 0.0;
  
  distancesToSolutionSet[solution] = distanceToSolutionSet(solutionVector, refSet1);
  aux = distanceToSolutionSet(solutionVector, refSet2);
  if (aux < distancesToSolutionSet[solution])
    distancesToSolutionSet[solution] = aux;
  
  if (refSet2->getNumSolutions() < refSet2Size_)
  {
    refSet2->insertSolution(solution, fitness);
    return true;
  }
  
  double peor = 0.0;
  size_t index = 0;
  for (size_t i = 0; i < refSet2->getNumSolutions(); ++i)
  {
    jassert(distancesToSolutionSet.find(refSet2->getSolution(i)) != distancesToSolutionSet.end());
    aux = distancesToSolutionSet[refSet2->getSolution(i)];
    if (aux > peor)
    {
      peor = aux;
      index = i;
    }
  }
  
  jassert(distancesToSolutionSet.find(solution) != distancesToSolutionSet.end());
  
  if (distancesToSolutionSet[solution] < peor)
  {
    refSet2->removeSolution(index);
    // Update distances in refSet2
    for (size_t i = 0; i < refSet2->getNumSolutions(); ++i)
    {
      aux = solutionVector->distanceTo(refSet2->getSolution(i).staticCast<DenseDoubleVector>());
      if (aux < distancesToSolutionSet[refSet2->getSolution(i)])
        distancesToSolutionSet[refSet2->getSolution(i)] = aux;
    }
    marked[solution] = false;
    refSet2->insertSolution(solution, fitness);
    return true;
  }
  return false;
}
  
size_t AbYSSOptimizer::subSetGeneration(ExecutionContext& context)
{
  std::vector<ObjectPtr> parents = std::vector<ObjectPtr>(2);
  std::vector<ObjectPtr> offspring;
  
  subSet->clear();
  
  // All pairs from refSet1
  for (size_t i = 0; i < refSet1->getNumSolutions(); ++i)
  {
    parents[0] = refSet1->getSolution(i);
    for (size_t j = i+1; j < refSet1->getNumSolutions(); ++j)
    {
      parents[1] = refSet1->getSolution(j);
      if (!marked[parents[0]] || !marked[parents[1]])
      {
        offspring = crossoverOperator->execute(context, problem, parents);
        subSet->insertSolution(offspring[0], evaluate(context, offspring[0]));
        subSet->insertSolution(offspring[1], evaluate(context, offspring[1]));
        marked[parents[0]] = true;
        marked[parents[1]] = true;
      }
    }
  }
  
  // All pairs from refSet2
  for (size_t i = 0; i < refSet2->getNumSolutions(); ++i)
  {
    parents[0] = refSet2->getSolution(i);
    for (size_t j = i+1; j < refSet2->getNumSolutions(); ++j)
    {
      parents[1] = refSet2->getSolution(j);
      if (!marked[parents[0]] || !marked[parents[1]])
      {
        offspring = crossoverOperator->execute(context, problem, parents);
        subSet->insertSolution(offspring[0], evaluate(context, offspring[0]));
        subSet->insertSolution(offspring[1], evaluate(context, offspring[1]));
        marked[parents[0]] = true;
        marked[parents[1]] = true;
      }
    }
  }
  
  return subSet->getNumSolutions();
}

} /* namespace lbcpp */
