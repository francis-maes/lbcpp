/*-----------------------------------------.---------------------------------.
| Filename: SharkProblems.h                | Wrapper for Shark               |
| Author  : Francis Maes                   |  single-objective and multi-obj.|
| Started : 12/09/2012 16:10               |  problems                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_SHARK_H_
# define LBCPP_MOO_PROBLEM_SHARK_H_

# include <lbcpp-ml/Problem.h>
# include <EALib/ObjectiveFunctions.h>
# include <EALib/MultiObjectiveFunctions.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class ObjectiveFromSharkObjectiveFunction : public Objective
{
public:
  ObjectiveFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective, size_t objectiveIndex, double worstScore, double bestScore)
    : objective(objective), objectiveIndex(objectiveIndex), worstScore(worstScore), bestScore(bestScore) {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = worstScore; best = bestScore;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    DenseDoubleVectorPtr vector = object.staticCast<DenseDoubleVector>();
    jassert(vector);
    std::vector<double> res;
    objective->result(vector->getValues(), res);
    jassert(objectiveIndex < res.size());
    return res[objectiveIndex];
  }

protected:
  ObjectiveFunctionVS<double>* objective;
  size_t objectiveIndex;
  double worstScore;
  double bestScore;
};

class ProblemFromSharkObjectiveFunction : public Problem
{
public:
  ProblemFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective)
    : objective(objective) {}

  virtual ~ProblemFromSharkObjectiveFunction()
    {delete objective;}

  virtual void initialize(ExecutionContext& context)
  {
    if (!domain)
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
        setDomain(new ContinuousDomain(limits));
      }
    }

    for (size_t i = 0; i < objective->objectives(); ++i)
    {
      double worst, best;
      getObjectiveRange(i, worst, best);
      addObjective(new ObjectiveFromSharkObjectiveFunction(objective, i, worst, best));
    }

    DenseDoubleVectorPtr initialGuess = new DenseDoubleVector(domain.staticCast<ContinuousDomain>()->getNumDimensions(), 0.0);
    objective->ProposeStartingPoint(initialGuess->getValues());
    setInitialGuess(initialGuess);
  }

  virtual string toShortString() const
  {
    string res = getClassName();
    int i = res.indexOf(T("Problem"));
    if (i >= 0)
      res = res.substring(0, i);
    return res;
  }

protected:
  ObjectiveFunctionVS<double>* objective;
  
  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const = 0;
};

/*
** Single-objective benchmark functions
*/
class SingleObjectiveSharkMOProblem : public ProblemFromSharkObjectiveFunction
{
public:
  SingleObjectiveSharkMOProblem(ObjectiveFunctionVS<double>* objective, double max = 10.0)
    : ProblemFromSharkObjectiveFunction(objective), max(max) {}
  
  virtual void adjustLimits(std::vector< std::pair<double, double> >& res) const
    {res[0].first = max;}

protected:
  double max;
};

struct AckleyProblem : public ProblemFromSharkObjectiveFunction
{
  AckleyProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(new Ackley((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 20.0; best = 0.0;}
};

struct GriewangkProblem : public ProblemFromSharkObjectiveFunction
{
  GriewangkProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(new Griewangk((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 400.0; best = 0.0;}
};

struct RastriginProblem : public ProblemFromSharkObjectiveFunction
{
  RastriginProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(new Rastrigin((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 400.0; best = 0.0;}
};

struct RosenbrockProblem : public ProblemFromSharkObjectiveFunction
{
  RosenbrockProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(new Rosenbrock((unsigned)numDimensions)), numDimensions(numDimensions)
    {initialize(defaultExecutionContext());}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ContinuousDomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0))));
    ProblemFromSharkObjectiveFunction::initialize(context);
  }

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 4000.0; best = 0.0;}

protected:
  size_t numDimensions;
};

struct RosenbrockRotatedProblem : public ProblemFromSharkObjectiveFunction
{
  RosenbrockRotatedProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(new RosenbrockRotated((unsigned)numDimensions)), numDimensions(numDimensions)
    {initialize(defaultExecutionContext());}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ContinuousDomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0))));
    ProblemFromSharkObjectiveFunction::initialize(context);
  }

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 10.0; best = 0.0;}

protected:
  size_t numDimensions;
};


/*
** ZDT Bi-objective functions
*/
class ZDTMOProblem : public ProblemFromSharkObjectiveFunction
{
public:
  ZDTMOProblem(ObjectiveFunctionVS<double>* objective, double max1, double max2)
    : ProblemFromSharkObjectiveFunction(objective), max1(10.0), max2(10.0)
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = objectiveIndex == 1 ? max2 : max1; best = 0.0;}

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
