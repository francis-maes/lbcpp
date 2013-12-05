/*-----------------------------------------.---------------------------------.
| Filename: SMPSOOptimizer.cpp             | SMPSO Optimizer                 |
| Author  : Denny Verbeeck                 |                                 |
| Started : 29/11/2013 14:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "SMPSOOptimizer.h"
#include <ml/Domain.h>
#include <ml/SolutionComparator.h>

namespace lbcpp
{

void SMPSOOptimizer::init(ExecutionContext& context)
{
  // Default configuration
  r1Max_ = 1.0;
  r1Min_ = 0.0;
  r2Max_ = 1.0;
  r2Min_ = 0.0;
  C1Max_ = 2.5;
  C1Min_ = 1.5;
  C2Max_ = 2.5;
  C2Min_ = 1.5;
  WMax_ = 0.1;
  WMin_ = 0.1;
  ChVel1_ = -1;
  ChVel2_ = -1;

  particles = new SolutionVector(problem->getFitnessLimits());
  particles->reserve(populationSize);
  best = new SolutionVector(problem->getFitnessLimits());
  best->reserve(populationSize);
  leaders = new CrowdingArchive(archiveSize, problem->getFitnessLimits());

  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  size_t numDimensions = domain->getNumDimensions();
  deltaMax_ = new double[numDimensions];
  deltaMin_ = new double[numDimensions];

  for (size_t i = 0; i < numDimensions; ++i)
  {
    deltaMax_[i] = (domain->getUpperLimit(i) - domain->getLowerLimit(i)) / 2.0;
    deltaMin_[i] = -deltaMax_[i];
  }

  speed = new double*[populationSize];
  for (size_t i = 0; i < populationSize; ++i)
  {
    speed[i] = new double[numDimensions];
    for (size_t j = 0; j < numDimensions; ++j)
      speed[i][j] = 0;
  }

  /* Set up mutation operator parameters */
  mutationProbability = 1.0 / domain->getNumDimensions();
  distributionIndex = 20.0;
  eta_m = 20.0;
}

bool SMPSOOptimizer::iterateSolver(ExecutionContext& context, size_t iter)
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
        best->setSolution(i, cloneVector(context, object), fitness);
    }
  }
  return true;
}

void SMPSOOptimizer::computeSpeed(ExecutionContext& context, size_t iter)
{
  double r1, r2, W, C1, C2;
  double wmax, wmin;
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
    r1 = context.getRandomGenerator()->sampleDouble(r1Min_, r1Max_);
    r2 = context.getRandomGenerator()->sampleDouble(r2Min_, r2Max_);
    C1 = context.getRandomGenerator()->sampleDouble(C1Min_, C1Max_);
    C2 = context.getRandomGenerator()->sampleDouble(C2Min_, C2Max_);
    W = context.getRandomGenerator()->sampleDouble(WMin_, WMax_);
    //
    wmax = WMax_;
    wmin = WMin_;

    for (size_t var = 0; var < particle->getNumValues(); ++var)
    {
      //Computing the velocity of this particle 
      speed[i][var] = velocityConstriction(constrictionCoefficient(C1, C2) *
        (inertiaWeight(iter, numIterations, wmax, wmin) *
        speed[i][var] +
        C1 * r1 * (bestParticle->getValue(var) -
        particle->getValue(var)) +
        C2 * r2 * (bestGlobal->getValue(var) -
        particle->getValue(var))), deltaMax_, //[var],
        deltaMin_, //[var], 
        var,
        i);
    }
  }
}

double SMPSOOptimizer::inertiaWeight(int iter, int miter, double wma, double wmin) {
  return wma; // - (((wma-wmin)*(double)iter)/(double)miter);
}

double SMPSOOptimizer::constrictionCoefficient(double c1, double c2) {
  double rho = c1 + c2; 
  if (rho <= 4)
    return 1.0;
  else
    return 2 / (2 - rho - sqrt(pow(rho, 2.0) - 4.0 * rho));
}

double SMPSOOptimizer::velocityConstriction(double v, double* deltaMax, double* deltaMin, int variableIndex, int particleIndex) {
  double result;
  double dmax = deltaMax[variableIndex];
  double dmin = deltaMin[variableIndex];
  result = v;

  if (v > dmax)
    result = dmax;
  if (v < dmin)
    result = dmin;
  return result;
}

void SMPSOOptimizer::computeNewPositions()
{
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  for (size_t i = 0; i < populationSize; ++i) 
  {
    DenseDoubleVectorPtr particle = particles->getSolution(i).staticCast<DenseDoubleVector>();
    for (size_t var = 0; var < particle->getNumValues(); ++var)
    {
      particle->setValue(var, particle->getValue(var) +  speed[i][var]);
      if (particle->getValue(var) < domain->getLowerLimit(var))
      {
        particle->setValue(var, domain->getLowerLimit(var));
        speed[i][var] = speed[i][var] * ChVel1_;
      }
      if (particle->getValue(var) > domain->getUpperLimit(var))
      {
        particle->setValue(var, domain->getUpperLimit(var));
        speed[i][var] = speed[i][var] * ChVel2_;
      }
    }
  }
}

void SMPSOOptimizer::mopsoMutation(ExecutionContext& context, size_t iter)
{
  for (size_t i = 0; i < particles->getNumSolutions(); ++i)
    if ((i % 6) == 0)
      doMutation(context, particles->getSolution(i).staticCast<DenseDoubleVector>()) ;
}

void SMPSOOptimizer::doMutation(ExecutionContext& context, DenseDoubleVectorPtr particle)
{
  double rnd, delta1, delta2, mut_pow, deltaq;
  double y, yl, yu, val, xy;
  ScalarVectorDomainPtr domain = problem->getDomain().staticCast<ScalarVectorDomain>();
  for (size_t var=0; var < particle->getNumValues(); ++var)
  {
    if (context.getRandomGenerator()->sampleDouble() <= mutationProbability)
	  {
		  y      = particle->getValue(var);
      yl     = domain->getLowerLimit(var);              
		  yu     = domain->getUpperLimit(var);
		  delta1 = (y-yl)/(yu-yl);
		  delta2 = (yu-y)/(yu-yl);
		  rnd = context.getRandomGenerator()->sampleDouble();
		  mut_pow = 1.0/(eta_m+1.0);
		  if (rnd <= 0.5)
		  {
			  xy     = 1.0-delta1;
			  val    = 2.0*rnd+(1.0-2.0*rnd)*(pow(xy,(distributionIndex+1.0)));
			  deltaq =  pow(val,mut_pow) - 1.0;
		  }
		  else
		  {
			  xy = 1.0-delta2;
			  val = 2.0*(1.0-rnd)+2.0*(rnd-0.5)*(pow(xy,(distributionIndex+1.0)));
			  deltaq = 1.0 - (pow(val,mut_pow));
		  }
		  y = y + deltaq*(yu-yl);
		  if (y<yl)
			  y = yl;
		  if (y>yu)
			  y = yu;
		  particle->setValue(var, y);                           
	  }
  }
}

void SMPSOOptimizer::cleanUp()
{
  if (deltaMin_)
    delete[] deltaMin_;
  if (deltaMax_)
    delete[] deltaMax_;
  if (speed)
  {
    for (size_t i = 0; i < populationSize; ++i)
      delete[] speed[i];
    delete[] speed;
  }
  deltaMin_ = 0;
  deltaMax_ = 0;
  speed = 0;
}

} /* namespace lbcpp */
