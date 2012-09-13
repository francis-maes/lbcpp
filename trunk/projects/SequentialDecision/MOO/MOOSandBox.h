/*-----------------------------------------.---------------------------------.
| Filename: MOOSandBox.h                   | Multi Objective Optimization    |
| Author  : Francis Maes                   | SandBox                         |
| Started : 11/09/2012 11:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_SANDBOX_H_
# define LBCPP_MOO_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "SharkMOOptimizers.h"
# include "RandomOptimizer.h"
# include "SharkMOProblems.h"
# include "DecoratorProblems.h"

namespace lbcpp
{

class UniformContinuousSampler : public MOOSampler
{
public:
  virtual ObjectPtr sample(ExecutionContext& context, const MOODomainPtr& dom) const
  {
    ContinuousMOODomainPtr domain = dom.staticCast<ContinuousMOODomain>();
    jassert(domain);
    return domain->sampleUniformly(context.getRandomGenerator());
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& solutions)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& solution)
    {jassertfalse;}
};

///////////////////////////////////////////////

class CrossEntropyOptimizer : public PopulationBasedMOOOptimizer
{
public:
  CrossEntropyOptimizer(MOOSamplerPtr sampler, size_t populationSize, size_t numTrainingSamples, size_t numGenerations = 0)
    : PopulationBasedMOOOptimizer(populationSize, numGenerations), sampler(sampler), numTrainingSamples(numTrainingSamples) {}
  CrossEntropyOptimizer() {}

  virtual void optimize(ExecutionContext& context)
  {
    MOOSamplerPtr sampler = this->sampler;
    for (size_t i = 0; (numGenerations == 0 || i < numGenerations) && !problem->shouldStop(); ++i)
    {
      MOOParetoFrontPtr population = sampleAndEvaluatePopulation(context, sampler, populationSize);
      std::vector<ObjectPtr> samples = selectTrainingSamples(context, population);
      sampler = sampler->cloneAndCast<MOOSampler>();
      sampler->learn(context, samples);
    }
  }

  virtual std::vector<ObjectPtr> selectTrainingSamples(ExecutionContext& context, MOOParetoFrontPtr population) const
  {
    // default implementation for single objective
    jassert(problem->getNumObjectives() == 1);

    std::vector< std::pair<MOOFitnessPtr, ObjectPtr> > pop;
    population->getSolutions(pop);
    bool isMaximisation = problem->getFitnessLimits()->shouldObjectiveBeMaximized(0);

    std::vector<ObjectPtr> res(numTrainingSamples < pop.size() ? numTrainingSamples : pop.size());
    for (size_t i = 0; i < res.size(); ++i)
      res[i] = pop[isMaximisation ? res.size() - 1 - i : i].second;
    return res;
  }

protected:
  friend class CrossEntropyOptimizerClass;

  MOOSamplerPtr sampler;
  size_t numTrainingSamples;
};

///////////////////////////////////////////////

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new ZDT1MOProblem());
    problems.push_back(new ZDT2MOProblem());
    problems.push_back(new ZDT3MOProblem());
    problems.push_back(new ZDT4MOProblem());
    problems.push_back(new ZDT6MOProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());

      context.resultCallback("problem", problem);
      solveWithOptimizer(context, problem, new RandomOptimizer(new UniformContinuousSampler()));
      solveWithOptimizer(context, problem, new NSGA2MOOptimizer(100));
      solveWithOptimizer(context, problem, new CMAESMOOptimizer(100, 100));
      solveWithOptimizer(context, problem, new CMAESMOOptimizer(5, 100));
      context.leaveScope();
    }
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  size_t numEvaluations;

  void solveWithOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorProblemPtr decorator(new HyperVolumeEvaluatorDecoratorProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

    context.enterScope(optimizer->toShortString());
    MOOParetoFrontPtr paretoFront = optimizer->optimize(context, decorator);
    context.resultCallback("optimizer", optimizer);
    context.resultCallback("paretoFront", paretoFront);
    context.resultCallback("numEvaluations", decorator->getNumEvaluations());

    //context.enterScope("HyperVolume Curve");
    std::vector<double> hyperVolumes = decorator->getHyperVolumes();
    std::vector<double> cpuTimes = decorator->getCpuTimes();

    for (size_t i = 0; i < hyperVolumes.size(); ++i)
    {
      size_t numEvaluations = i * decorator->getHyperVolumeComputationPeriod();
      context.enterScope(String((int)numEvaluations));
      context.resultCallback("numEvaluations", numEvaluations);
      context.resultCallback("hyperVolume", hyperVolumes[i]);
      context.resultCallback("cpuTime", cpuTimes[i]);
      context.leaveScope();
    }
    //context.leaveScope();

    context.leaveScope(paretoFront->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
