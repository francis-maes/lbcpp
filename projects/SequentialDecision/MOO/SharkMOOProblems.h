/*-----------------------------------------.---------------------------------.
| Filename: SharkMOOProblems.h             | Wrapper for Shark MOO Problems  |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 16:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_SHARK_H_
# define LBCPP_MOO_PROBLEM_SHARK_H_

# include "MOOCore.h"
# include <EALib/MultiObjectiveFunctions.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class MOOProblemFromSharkObjectiveFunction : public MOOProblem
{
public:
  MOOProblemFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective)
    : objective(objective)
  {
     const BoxConstraintHandler* box = static_cast<const BoxConstraintHandler*>(objective->getConstraintHandler());
    jassert(box);
    std::vector< std::pair<double, double> > limits(box->dimension());
    for (size_t i = 0; i < limits.size(); ++i)
    {
      limits[i].first = box->lowerBound((unsigned int)i);
      limits[i].second = box->upperBound((unsigned int)i);
    }
    domain = new ContinuousMOODomain(limits);
  }

  virtual ~MOOProblemFromSharkObjectiveFunction()
    {delete objective;}

  virtual MOODomainPtr getSolutionDomain() const
    {return domain;}

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
  {
    if (!limits)
    {
      std::vector< std::pair<double, double> > v(objective->objectives(), std::make_pair(DBL_MAX, 0.0));
      adjustLimits(v);
      const_cast<MOOProblemFromSharkObjectiveFunction* >(this)->limits = new MOOFitnessLimits(v);
    }
    return limits;
  }

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    DenseDoubleVectorPtr vector = solution.staticCast<DenseDoubleVector>();
    jassert(vector);
    std::vector<double> res;
    objective->result(vector->getValues(), res);
    return new MOOFitness(res, limits);
  }

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(domain->getNumDimensions(), 0.0);
    objective->ProposeStartingPoint(res->getValues());
    return res;
  }

protected:
  ContinuousMOODomainPtr domain;
  MOOFitnessLimitsPtr limits;
  ObjectiveFunctionVS<double>* objective;

  virtual void adjustLimits(std::vector< std::pair<double, double> >& res) const {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PROBLEM_SHARK_H_
