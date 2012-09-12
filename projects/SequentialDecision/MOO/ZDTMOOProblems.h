/*-----------------------------------------.---------------------------------.
| Filename: ZDTMOOProblems.h               | ZDT Multi Objective Optimization|
| Author  : Francis Maes                   | Benchmarks                      |
| Started : 12/09/2012 15:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_ZDT_H_
# define LBCPP_MOO_PROBLEM_ZDT_H_

# include "MOOCore.h"
# include <MOO-EALib/TestFunction.h>

namespace lbcpp
{
#if 0
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
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PROBLEM_ZDT_H_
