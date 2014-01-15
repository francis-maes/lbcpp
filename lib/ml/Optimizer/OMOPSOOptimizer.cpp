/*-----------------------------------------.---------------------------------.
| Filename: OMOPSOOptimizer.h              | OMOPSO Optimizer                |
| Author  : Denny Verbeeck                 |                                 |
| Started : 03/12/2013 23:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "OMOPSOOptimizer.h"

namespace lbcpp
{

void OMOPSOOptimizer::init(ExecutionContext& context)
{
  particles = new SolutionVector(problem->getFitnessLimits());
  particles->reserve(populationSize);
  best = new SolutionVector(problem->getFitnessLimits());
  best->reserve(populationSize);
  leaders = new CrowdingArchive(archiveSize, problem->getFitnessLimits());

  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  size_t numDimensions = domain->getNumDimensions();
  speed = new double*[populationSize];
  for (size_t i = 0; i < populationSize; ++i)
  {
    speed[i] = new double[numDimensions];
    for (size_t j = 0; j < numDimensions; ++j)
      speed[i][j] = 0;
  }

  uniformMutationProbability = 1.0 / numDimensions;
  nonUniformMutationProbability = 1.0 / numDimensions;
  uniformMutationPerturbationIndex = 0.5;
  nonUniformMutationPerturbationIndex = 0.5;
  uMutation = uniformMutation(uniformMutationProbability, uniformMutationPerturbationIndex);
}

bool OMOPSOOptimizer::iterateSolver(ExecutionContext& context, size_t iter)
{
  DenseDoubleVectorPtr object;
  FitnessPtr fitness;
  if (iter == 0)
  {
    /* Create initial population */
    initialVectorSampler->initialize(context, new VectorDomain(problem->getDomain()));
    OVectorPtr initialSamples = initialVectorSampler->sample(context).staticCast<OVector>();
    jassert(initialSamples->getNumElements() == populationSize);
    for (size_t i = 0; i < initialSamples->getNumElements(); ++i)
    {
      object = initialSamples->getAndCast<DenseDoubleVector>(i);
      fitness = evaluate(context, object);
      particles->insertSolution(cloneVector(context, object), fitness);
      leaders->insertSolution(object, fitness);
      best->insertSolution(object, fitness);
    }
  }
  else
  {
    computeSpeed(context, iter);
    computeNewPositions();
    mopsoMutation(context, iter);
    for (size_t i = 0; i < populationSize; ++i)
    {
      object = particles->getSolution(i).staticCast<DenseDoubleVector>();
      fitness = evaluate(context, object);
      leaders->insertSolution(cloneVector(context, object), fitness);
      if (fitness->strictlyDominates(best->getFitness(i)))
        best->setSolution(i, cloneVector(context, object), new Fitness(*fitness));
    }
  }
  return true;
}

void OMOPSOOptimizer::computeSpeed(ExecutionContext& context, size_t iter)
{
  double r1, r2, W, C1, C2;
  DenseDoubleVectorPtr bestGlobal;
  SolutionComparatorPtr comparator = paretoRankAndCrowdingDistanceComparator();
  comparator->initialize(leaders);

  for (size_t i = 0; i < populationSize; ++i)
  {
    DenseDoubleVectorPtr particle = particles->getSolution(i);
    DenseDoubleVectorPtr bestParticle = best->getSolution(i);                       

    //Select a global best_ for calculate the speed of particle i, bestGlobal
    if (leaders->getNumSolutions() > 1)
    {
      DenseDoubleVectorPtr one, two;
      int pos1 = context.getRandomGenerator()->sampleInt(leaders->getNumSolutions() - 1);
      int pos2 = context.getRandomGenerator()->sampleInt(leaders->getNumSolutions() - 1);

      if (comparator->compareSolutions(one, two) < 1)
        bestGlobal = leaders->getSolution(pos1);
      else
        bestGlobal = leaders->getSolution(pos2);
    }
    else
      bestGlobal = leaders->getSolution(0);
            
    //Params for velocity equation
    r1 = context.getRandomGenerator()->sampleDouble();
    r2 = context.getRandomGenerator()->sampleDouble();
    C1 = context.getRandomGenerator()->sampleDouble(1.5,2.0);
    C2 = context.getRandomGenerator()->sampleDouble(1.5,2.0);
    W  = context.getRandomGenerator()->sampleDouble(0.1,0.5);            
    //

    for (size_t var = 0; var < particle->getNumValues(); ++var)
      //Computing the velocity of this particle
      speed[i][var] = W  * speed[i][var] +
        C1 * r1 * (bestParticle->getValue(var) - particle->getValue(var)) +
        C2 * r2 * (bestGlobal->getValue(var) - particle->getValue(var));
  }
}

void OMOPSOOptimizer::computeNewPositions()
{
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  for (size_t i = 0; i < populationSize; ++i)
  {
    DenseDoubleVectorPtr particle = particles->getSolution(i);
    for (size_t var = 0; var < particle->getNumValues(); ++var)
    {
      particle->setValue(var, particle->getValue(var) + speed[i][var]);
      if (particle->getValue(var) < domain->getLowerLimit(var))
      {
        particle->setValue(var, domain->getLowerLimit(var));
        speed[i][var] = speed[i][var] * -1.0;
      }
      if (particle->getValue(var) > domain->getUpperLimit(var))
      {
        particle->setValue(var, domain->getUpperLimit(var));
        speed[i][var] = speed[i][var] * -1.0;
      }                                             
    }
  }
}

void OMOPSOOptimizer::mopsoMutation(ExecutionContext& context, size_t iter)
{
  for (size_t i = 0; i < particles->getNumSolutions(); ++i)
    if ((i % 3) == 0)
    {
      MutationPtr nuMutation = nonUniformMutation(nonUniformMutationProbability, nonUniformMutationPerturbationIndex, iter, numIterations);
      nuMutation->execute(context, problem, particles->getSolution(i));
    }
    else if ((i % 3) == 1)
      uMutation->execute(context, problem, particles->getSolution(i));
}

void OMOPSOOptimizer::cleanUp()
{
  if (speed)
  {
    for (size_t i = 0; i < populationSize; ++i)
      delete[] speed[i];
    delete[] speed;
    speed = 0;
  }
}

} /* namespace lbcpp */
