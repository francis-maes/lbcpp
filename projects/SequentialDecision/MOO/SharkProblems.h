/*-----------------------------------------.---------------------------------.
| Filename: SharkProblems.h                | Wrapper for Shark               |
| Author  : Francis Maes                   |  single-objective and multi-obj.|
| Started : 12/09/2012 16:10               |  problems                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_SHARK_H_
# define LBCPP_MOO_PROBLEM_SHARK_H_

# include "MOOCore.h"
# include <EALib/ObjectiveFunctions.h>
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
    if (box)
    {
      std::vector< std::pair<double, double> > limits(box->dimension());
      for (size_t i = 0; i < limits.size(); ++i)
      {
        limits[i].first = box->lowerBound((unsigned int)i);
        limits[i].second = box->upperBound((unsigned int)i);
      }
      domain = new ContinuousMOODomain(limits);
    }
  }

  virtual ~MOOProblemFromSharkObjectiveFunction()
    {delete objective;}

  virtual MOODomainPtr getObjectDomain() const
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

  virtual String toShortString() const
  {
    String res = getClassName();
    int i = res.indexOf(T("MOOProblem"));
    if (i >= 0)
      res = res.substring(0, i);
    return res;
  }

protected:
  ContinuousMOODomainPtr domain;
  MOOFitnessLimitsPtr limits;
  ObjectiveFunctionVS<double>* objective;

  virtual void adjustLimits(std::vector< std::pair<double, double> >& res) const {}
};

/*
** Single-objective benchmark functions
*/
class SingleObjectiveSharkMOProblem : public MOOProblemFromSharkObjectiveFunction
{
public:
  SingleObjectiveSharkMOProblem(ObjectiveFunctionVS<double>* objective, double max = 10.0)
    : MOOProblemFromSharkObjectiveFunction(objective), max(max) {}
  
  virtual void adjustLimits(std::vector< std::pair<double, double> >& res) const
    {res[0].first = max;}

protected:
  double max;
};

struct AckleyProblem : public SingleObjectiveSharkMOProblem
  { AckleyProblem(size_t numDimensions = 30) : SingleObjectiveSharkMOProblem(new Ackley((unsigned)numDimensions), 20.0) {} };

struct GriewangkProblem : public SingleObjectiveSharkMOProblem
  { GriewangkProblem(size_t numDimensions = 30) : SingleObjectiveSharkMOProblem(new Griewangk((unsigned)numDimensions), 400.0) {} };

struct RastriginProblem : public SingleObjectiveSharkMOProblem
  { RastriginProblem(size_t numDimensions = 30) : SingleObjectiveSharkMOProblem(new Rastrigin((unsigned)numDimensions), 400.0) {} };

struct RosenbrockProblem : public SingleObjectiveSharkMOProblem
{
  RosenbrockProblem(size_t numDimensions = 30)
    : SingleObjectiveSharkMOProblem(new Rosenbrock((unsigned)numDimensions), 4000.0) 
  {
    domain = new ContinuousMOODomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0)));
  }
};

struct RosenbrockRotatedProblem : public SingleObjectiveSharkMOProblem
{
  RosenbrockRotatedProblem(size_t numDimensions = 30)
    : SingleObjectiveSharkMOProblem(new RosenbrockRotated((unsigned)numDimensions))
  {
    domain = new ContinuousMOODomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0)));
  }
};


/*
** ZDT Bi-objective functions
*/
class ZDTMOProblem : public MOOProblemFromSharkObjectiveFunction
{
public:
  ZDTMOProblem(ObjectiveFunctionVS<double>* objective, double max1, double max2)
    : MOOProblemFromSharkObjectiveFunction(objective), max1(10.0), max2(10.0) {} // TMP !!

  virtual void adjustLimits(std::vector< std::pair<double, double> >& res) const
    {res[0].first = max1; res[1].first = max2;}

private:
  double max1;
  double max2;
};

struct ZDT1MOProblem : public ZDTMOProblem {ZDT1MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT1(numDimensions), 1.0, 1.0) {} };
struct ZDT2MOProblem : public ZDTMOProblem {ZDT2MOProblem() : ZDTMOProblem(new ZDT2(30), 1.0, 1.0) {} };
struct ZDT3MOProblem : public ZDTMOProblem {ZDT3MOProblem() : ZDTMOProblem(new ZDT3(30), 0.852, 1.0) {} };
struct ZDT4MOProblem : public ZDTMOProblem {ZDT4MOProblem() : ZDTMOProblem(new ZDT4(30), 1.0, 1.0) {} };
// ZDT5 not wrapper yet (domain is discrete)
struct ZDT6MOProblem : public ZDTMOProblem {ZDT6MOProblem() : ZDTMOProblem(new ZDT6(30), 1.0, 1.0) {} };

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PROBLEM_SHARK_H_
