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
# include "SharkMOOOptimizer.h"
# include "RandomMOOOptimizer.h"
# include "ZDTMOOProblems.h"
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
    context.leaveScope();

    context.leaveScope(paretoFront);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_SANDBOX_H_
