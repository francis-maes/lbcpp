/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SANDBOX_H_
# define LBCPP_MOO_SANDBOX_H_

# include "MOOCore.h"
# include <lbcpp/Execution/WorkUnit.h>
# include <MOO-EALib/TestFunction.h>

namespace lbcpp
{

class RandomMOOOptimizer : public MOOOptimizer
{
public:
  RandomMOOOptimizer(MOOSamplerPtr sampler, size_t numIterations)
    : sampler(sampler), numIterations(numIterations) {}
  RandomMOOOptimizer() : numIterations(0) {}

  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet)
  {
    for (size_t iteration = 0; iteration < numIterations && !problem->shouldStop(); ++iteration)
    {
      ObjectPtr solution = sampler->sample(context, problem->getSolutionDomain());
      paretoSet->insert(solution, problem->evaluate(solution));
    }
  }

protected:
  friend class RandomMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numIterations;
};

class NRPAMOOOptimizer : public MOOOptimizer
{
public:
  NRPAMOOOptimizer(MOOSamplerPtr sampler, size_t level, size_t numIterationsPerLevel)
    : sampler(sampler), level(level), numIterationsPerLevel(numIterationsPerLevel) {}
  NRPAMOOOptimizer() : level(0), numIterationsPerLevel(0) {}
  
  virtual void optimize(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet)
    {optimizeRecursively(context, problem, paretoSet, this->sampler->cloneAndCast<MOOSampler>(), level);}

protected:
  friend class NRPAMOOOptimizerClass;

  MOOSamplerPtr sampler;
  size_t level;
  size_t numIterationsPerLevel;

  SolutionAndFitnessPair optimizeRecursively(ExecutionContext& context, MOOProblemPtr problem, MOOParetoFrontPtr paretoSet, MOOSamplerPtr sampler, size_t level)
  {
    if (problem->shouldStop())
      return SolutionAndFitnessPair();
      
    if (level == 0)
    {
      ObjectPtr solution = sampler->sample(context, problem->getSolutionDomain());
      return SolutionAndFitnessPair(solution, problem->evaluate(solution));
    }
    else
    {
      MOOFitnessPtr bestFitness = problem->getFitnessLimits()->getWorstPossibleFitness();
      ObjectPtr bestSolution;
        
      for (size_t i = 0; i < numIterationsPerLevel; ++i)
      {
        SolutionAndFitnessPair subResult = optimizeRecursively(context, problem, paretoSet, sampler->cloneAndCast<MOOSampler>(), level - 1);
        if (subResult.second->dominates(bestFitness))
        {
          bestSolution = subResult.first;
          bestFitness = subResult.second;
        }
        
        if (bestSolution)
          sampler->reinforce(bestSolution);
      }
      return std::make_pair(bestSolution, bestFitness);
    }
  }
};

//////////////////////////////////////

class ZDTMOOProblem : public MOOProblem
{
public:
  ZDTMOOProblem()
  {
    limits = new MOOFitnessLimits();
    limits->addObjective(1, 0); // f1
    limits->addObjective(DBL_MAX, 0); // f2
  }

  virtual MOODomainPtr getSolutionDomain() const
  {
    if (!domain)
      const_cast<ZDTMOOProblem* >(this)->domain = new ContinuousMOODomain(getDomainLimits());
    return domain;
  }

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {jassert(limits); return limits;}
  
  virtual MOOFitnessPtr evaluate(const ObjectPtr& solution) const
  {
    const std::vector<double>& sol = solution.staticCast<DenseDoubleVector>()->getValues();
    std::vector<double> objectives(2);
    objectives[0] = evaluateF1(sol);
    objectives[1] = evaluateF2(sol);
    return new MOOFitness(objectives, limits);
  }

protected:
  MOODomainPtr domain;
  MOOFitnessLimitsPtr limits;

  virtual std::vector< std::pair<double, double> > getDomainLimits() const = 0;
  virtual double evaluateF1(const std::vector<double>& solution) const = 0;
  virtual double evaluateF2(const std::vector<double>& solution) const = 0;
};

class ZTD1MOOProblem : public ZDTMOOProblem
{
protected:
  virtual std::vector< std::pair<double, double> > getDomainLimits() const
    {return std::vector<std::pair<double, double> >(30, std::make_pair(0.0, 1.0));}

  virtual double evaluateF1(const std::vector<double>& solution) const
    {return ZDT1F1(solution);}

  virtual double evaluateF2(const std::vector<double>& solution) const
    {return ZDT1F2(solution);}
};

///////////////////////////////////////////////

class UniformContinuousMOOSampler : public MOOSampler
{
public:
  virtual ObjectPtr sample(ExecutionContext& context, const MOODomainPtr& dom) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();
    ContinuousMOODomainPtr domain = dom.staticCast<ContinuousMOODomain>();
    jassert(domain);
    size_t n = domain->getNumDimensions();
    DenseDoubleVectorPtr res(new DenseDoubleVector(n, 0.0));
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, random->sampleDouble(domain->getLowerLimit(i), domain->getUpperLimit(i)));
    return res;
  }

  virtual void reinforce(const ObjectPtr& solution)
    {jassertfalse;}
};

///////////////////////////////////////////////

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : problem(new ZTD1MOOProblem()), numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    MOOSamplerPtr sampler = new UniformContinuousMOOSampler();
    MOOOptimizerPtr optimizer = new RandomMOOOptimizer(sampler, numEvaluations);
    MOOParetoFrontPtr paretoFront = optimizer->optimize(context, problem);

    context.resultCallback("problem", problem);
    context.resultCallback("sampler", sampler);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("paretoFront", paretoFront);

    std::vector< std::pair<MOOFitnessPtr, ObjectPtr> > solutions;
    paretoFront->getSolutions(solutions);
    context.enterScope("solutions");
    for (size_t i = 0; i < solutions.size(); ++i)
    {
      context.enterScope("Solution " + String((int)i+1));
      context.resultCallback("index", i+1);
      context.resultCallback("solution", solutions[i].second);
      MOOFitnessPtr fitness = solutions[i].first;
      context.resultCallback("fitness", fitness);
      for (size_t j = 0; j < fitness->getNumObjectives(); ++j)
        context.resultCallback("fitness" + String((int)j+1), fitness->getObjective(j));
      context.leaveScope();
    }
    context.leaveScope();
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  MOOProblemPtr problem;
  size_t numEvaluations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
