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
# include "SharkMOOOptimizers.h"
# include "RandomMOOOptimizer.h"
# include "SharkMOOProblems.h"
# include "DecoratorMOOProblems.h"

namespace lbcpp
{

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

class MOOSandBox : public WorkUnit
{
public:
  MOOSandBox() : numEvaluations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    std::vector<MOOProblemPtr> problems;
    problems.push_back(new ZDT1MOOProblem());
    problems.push_back(new ZDT2MOOProblem());
    problems.push_back(new ZDT3MOOProblem());
    problems.push_back(new ZDT4MOOProblem());
    problems.push_back(new ZDT6MOOProblem());

    for (size_t i = 0; i < problems.size(); ++i)
    {
      MOOProblemPtr problem = problems[i];
      context.enterScope(problem->toShortString());

      context.resultCallback("problem", problem);
      solveWithOptimizer(context, problem, new RandomMOOOptimizer(new UniformContinuousMOOSampler()));
      solveWithOptimizer(context, problem, new NSGA2MOOOptimizer(100));
      solveWithOptimizer(context, problem, new CMAESMOOOptimizer(100, 100));
      solveWithOptimizer(context, problem, new CMAESMOOOptimizer(5, 100));
      context.leaveScope();
    }
    return true;
  }

protected:
  friend class MOOSandBoxClass;

  size_t numEvaluations;

  void solveWithOptimizer(ExecutionContext& context, MOOProblemPtr problem, MOOOptimizerPtr optimizer)
  {
    HyperVolumeEvaluatorDecoratorMOOProblemPtr decorator(new HyperVolumeEvaluatorDecoratorMOOProblem(problem, numEvaluations, numEvaluations > 250 ? numEvaluations / 250 : 1));

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
