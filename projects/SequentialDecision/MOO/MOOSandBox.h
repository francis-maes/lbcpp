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
# include "MOOSharkOptimizer.h"
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
      paretoSet->insert(solution, problem->evaluate(context, solution));
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
      return SolutionAndFitnessPair(solution, problem->evaluate(context, solution));
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
    limits->addObjective(1.0, 0.0); // f1
    limits->addObjective(1.0, 0.0); // f2 (in fact it is ]+oo, 0], but we keep 1 as reference point for the hyper-volume calculation)
  }

  virtual MOODomainPtr getSolutionDomain() const
  {
    if (!domain)
      const_cast<ZDTMOOProblem* >(this)->domain = new ContinuousMOODomain(getDomainLimits());
    return domain;
  }

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {jassert(limits); return limits;}
  
  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    const std::vector<double>& sol = solution.staticCast<DenseDoubleVector>()->getValues();
    std::vector<double> objectives(2);
    objectives[0] = evaluateF1(sol);
    objectives[1] = evaluateF2(sol);
    return new MOOFitness(objectives, limits);
  }

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return domain->sampleUniformly(context.getRandomGenerator());}

protected:
  ContinuousMOODomainPtr domain;
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
    ContinuousMOODomainPtr domain = dom.staticCast<ContinuousMOODomain>();
    jassert(domain);
    return domain->sampleUniformly(context.getRandomGenerator());
  }

  virtual void reinforce(const ObjectPtr& solution)
    {jassertfalse;}
};

///////////////////////////////////////////////

class DecoratorMOOProblem : public MOOProblem
{
public:
  DecoratorMOOProblem(MOOProblemPtr problem = MOOProblemPtr())
    : problem(problem) {}

  virtual MOODomainPtr getSolutionDomain() const
    {return problem->getSolutionDomain();}

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {return problem->getFitnessLimits();}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {return problem->evaluate(context, solution);}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return problem->proposeStartingSolution(context);}

  virtual bool shouldStop() const
    {return problem->shouldStop();}

protected:
  friend class DecoratorMOOProblemClass;

  MOOProblemPtr problem;
};

class MaxNumIterationsDecoratorMOOProblem : public DecoratorMOOProblem
{
public:
  MaxNumIterationsDecoratorMOOProblem(MOOProblemPtr problem, size_t maxNumEvaluations)
    : DecoratorMOOProblem(problem), maxNumEvaluations(maxNumEvaluations), numEvaluations(0) {}
  MaxNumIterationsDecoratorMOOProblem() : maxNumEvaluations(0), numEvaluations(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {jassert(numEvaluations < maxNumEvaluations); ++numEvaluations; return DecoratorMOOProblem::evaluate(context, solution);}

  virtual bool shouldStop() const
    {return numEvaluations >= maxNumEvaluations || DecoratorMOOProblem::shouldStop();}

  size_t getNumEvaluations() const
    {return numEvaluations;}

protected:
  friend class MaxNumIterationsDecoratorMOOProblemClass;

  size_t maxNumEvaluations;

  size_t numEvaluations;
};

class HyperVolumeEvaluatorDecoratorMOOProblem : public MaxNumIterationsDecoratorMOOProblem
{
public:
  HyperVolumeEvaluatorDecoratorMOOProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t hyperVolumeComputationPeriod)
    : MaxNumIterationsDecoratorMOOProblem(problem, maxNumEvaluations), hyperVolumeComputationPeriod(hyperVolumeComputationPeriod)
    {front = new MOOParetoFront(problem->getFitnessLimits());}
  HyperVolumeEvaluatorDecoratorMOOProblem() : hyperVolumeComputationPeriod(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    MOOFitnessPtr res = MaxNumIterationsDecoratorMOOProblem::evaluate(context, solution);
    front->insert(solution, res);
    if ((numEvaluations % hyperVolumeComputationPeriod) == 0)
      hyperVolumes.push_back(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
    return res;
  }

  const std::vector<double>& getHyperVolumes() const
    {return hyperVolumes;}

  size_t getHyperVolumeComputationPeriod() const
    {return hyperVolumeComputationPeriod;}

protected:
  friend class HyperVolumeEvaluatorDecoratorMOOProblemClass;

  size_t hyperVolumeComputationPeriod;

  MOOParetoFrontPtr front;
  std::vector<double> hyperVolumes;
};

typedef ReferenceCountedObjectPtr<HyperVolumeEvaluatorDecoratorMOOProblem> HyperVolumeEvaluatorDecoratorMOOProblemPtr;

////////////////////////////////////////

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : problem(new ZTD1MOOProblem()), numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.resultCallback("problem", problem);
    solveWithOptimizer(context, new RandomMOOOptimizer(new UniformContinuousMOOSampler(), numEvaluations));
    solveWithOptimizer(context, new NSGA2MOOOptimizer(100, numEvaluations / 100));
    solveWithOptimizer(context, new CMAESMOOOptimizer(100, 100, numEvaluations / 100));
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  MOOProblemPtr problem;
  size_t numEvaluations;

  void solveWithOptimizer(ExecutionContext& context, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorMOOProblemPtr decorator(new HyperVolumeEvaluatorDecoratorMOOProblem(problem, numEvaluations, 1000));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr paretoFront = optimizer->optimize(context, decorator);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("paretoFront", paretoFront);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    context.enterScope("HyperVolume Curve");
    std::vector<double> hyperVolumes = decorator->getHyperVolumes();
    for (size_t i = 0; i < hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getHyperVolumeComputationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes[i]);
      context.leaveScope();
    }
    context.leaveScope();

    context.leaveScope(paretoFront);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
