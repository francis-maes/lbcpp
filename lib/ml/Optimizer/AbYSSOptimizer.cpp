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

  archiveSize_ = 100;
  refSet1Size_ = 10;
  refSet2Size_ = 10;

  size_t numDimensions = problem->getNumVariables();
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
  delete[] frequency_;
  delete[] reverseFrequency_;
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
    referenceSetUpdate(true);
    newSolutions = subSetGeneration();
    while (newSolutions > 0)  // New solutions are created
    {
      referenceSetUpdate(false);
      newSolutions = subSetGeneration();
    }

    // RE-START
    solutionSet->clear();
    for (size_t i = 0; i < refSet1->getNumSolutions(); ++i)
    {
      object = refSet1->getSolution(i).staticCast<DenseDoubleVector>();
      // set object unmarked
      improvementOperator->execute(context, problem, object);
      fitness = evaluate(context, object);
      solutionSet->insertSolution(object, fitness);
    }

    refSet1->clear();
    refSet2->clear();

    size_t insert = solutionSetSize_ / 2;
    if (insert > archive->getNumSolutions())
      insert = archive->getNumSolutions();

    if (insert > solutionSetSize_ - solutionSet->getNumSolutions())
      insert = solutionSetSize_ - solutionSet->getNumSolutions();

    // Insert solutions
    for (size_t i = 0; i < insert; ++i)
    {
      object = cloneVector(context, archive->getSolution(i).staticCast<DenseDoubleVector>());
      // set object unmarked
      fitness = archive->getFitness(i);
      solutionSet->insertSolution(object, fitness);
    }

    // Create the rest of solutions randomly
    while (solutionSet->getNumSolutions() < solutionSetSize_)
    {
      object = diversificationGeneration(context);
      improvementOperator->execute(context, problem, object);
      fitness = evaluate(context, object);
      // set object unmarked
      solutionSet->insertSolution(object, fitness);
    }
  }
  return true;
}

DenseDoubleVectorPtr AbYSSOptimizer::diversificationGeneration(ExecutionContext& context)
{
  DenseDoubleVectorPtr solution = new DenseDoubleVector(problem->getNumVariables(), 0.0);
  double value = 0.0;
  int range = 0;

  for (size_t i = 0; i < problem->getNumVariables(); ++i)
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

void AbYSSOptimizer::referenceSetUpdate(bool build)
{
  if (build)
  {
    ObjectPtr individual;
    FitnessPtr fitness;
    // Build a new reference set
    solutionSet->sort(spea2Comparator());
    for (size_t i = 0; i < refSet1Size_; ++i)
    {
      individual = solutionSet->getSolution(0);
      fitness = solutionSet->getFitness(0);
      solutionSet->removeSolution(0);
      // set individual unmarked
      refSet1->insertSolution(individual, fitness);
    }
  }

}

size_t AbYSSOptimizer::subSetGeneration()
{
  return 0;
}

} /* namespace lbcpp */
