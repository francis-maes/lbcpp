/*-----------------------------------------.---------------------------------.
| Filename: SharkProblems.h                | Wrapper for Shark               |
| Author  : Francis Maes                   |  single-objective and multi-obj.|
| Started : 12/09/2012 16:10               |  problems                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_PROBLEM_SHARK_H_
# define MOO_PROBLEM_SHARK_H_

# include <ml/Problem.h>
# include <EALib/ObjectiveFunctions.h>
# include <EALib/MultiObjectiveFunctions.h>
# include <ml/DoubleVector.h>

namespace lbcpp
{

class ObjectiveFromSharkObjectiveFunction : public Objective
{
public:
  ObjectiveFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective, size_t objectiveIndex, double worstScore, double bestScore, DenseDoubleVectorPtr optimum)
    : objective(objective), objectiveIndex(objectiveIndex), worstScore(worstScore), bestScore(bestScore), optimum(optimum) {}

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = worstScore; best = bestScore;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr vector = object.staticCast<DenseDoubleVector>();
    jassert(vector);
    std::vector<double> res;
    std::vector<double> point = vector->getValues();
    jassert(point.size() == optimum->getNumValues());
    for (size_t i = 0; i < point.size(); ++i)
      point[i] -= optimum->getValue(i);
    objective->result(point, res);
    jassert(objectiveIndex < res.size());
    return res[objectiveIndex];
  }

protected:
  ObjectiveFunctionVS<double>* objective;
  size_t objectiveIndex;
  double worstScore;
  double bestScore;
  DenseDoubleVectorPtr optimum;
};

class ProblemFromSharkObjectiveFunction : public Problem
{
public:
  ProblemFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective) 
    : numDimensions(objective->dimension()), objective(objective) {}
  ProblemFromSharkObjectiveFunction(size_t numDimensions, ObjectiveFunctionVS<double>* objective)
    : numDimensions(numDimensions), objective(objective) {}

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
        setDomain(new ScalarVectorDomain(limits));
      }
    }

    ScalarVectorDomainPtr scalarVectorDomain = getDomain().staticCast<ScalarVectorDomain>();
    DenseDoubleVectorPtr optimum = scalarVectorDomain->sampleUniformly(context.getRandomGenerator()).staticCast<DenseDoubleVector>();

    for (size_t i = 0; i < objective->objectives(); ++i)
    {
      double worst, best;
      getObjectiveRange(i, worst, best);
      addObjective(new ObjectiveFromSharkObjectiveFunction(objective, i, worst, best, optimum));
    }

    DenseDoubleVectorPtr initialGuess = new DenseDoubleVector(domain.staticCast<ScalarVectorDomain>()->getNumDimensions(), 0.0);
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
  friend class ProblemFromSharkObjectiveFunctionClass;
  size_t numDimensions;
  ObjectiveFunctionVS<double>* objective;
  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const = 0;
};

/*
** Single-objective benchmark functions
*/
struct SphereProblem : public ProblemFromSharkObjectiveFunction
{
  SphereProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new Sphere((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 4.0 * numDimensions; best = 0.0;}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0))));
    if (objective)
      delete objective;
    objective = new Sphere((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }
};

struct AckleyProblem : public ProblemFromSharkObjectiveFunction
{
  AckleyProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new Ackley((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 20.0; best = 0.0;}

  virtual void initialize(ExecutionContext& context)
  {
    if (objective)
      delete objective;
    objective = new Ackley((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }
};

struct GriewangkProblem : public ProblemFromSharkObjectiveFunction
{
  GriewangkProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new Griewangk((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 400.0; best = 0.0;}

  virtual void initialize(ExecutionContext& context)
  {
    if (objective)
      delete objective;
    objective = new Griewangk((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }
};

struct RastriginProblem : public ProblemFromSharkObjectiveFunction
{
  RastriginProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new Rastrigin((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 400.0; best = 0.0;}

  virtual void initialize(ExecutionContext& context)
  {
    if (objective)
      delete objective;
    objective = new Rastrigin((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }
};

struct RosenbrockProblem : public ProblemFromSharkObjectiveFunction
{
  RosenbrockProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new Rosenbrock((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0))));
    if (objective)
      delete objective;
    objective = new Rosenbrock((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 4000.0; best = 0.0;}
};

struct RosenbrockRotatedProblem : public ProblemFromSharkObjectiveFunction
{
  RosenbrockRotatedProblem(size_t numDimensions = 30) : ProblemFromSharkObjectiveFunction(numDimensions, new RosenbrockRotated((unsigned)numDimensions))
    {initialize(defaultExecutionContext());}

  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(numDimensions, std::make_pair(-2.0, 2.0))));
    if (objective)
      delete objective;
    objective = new RosenbrockRotated((unsigned)numDimensions);
    ProblemFromSharkObjectiveFunction::initialize(context);
  }

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = 10.0; best = 0.0;}
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

#endif // !MOO_PROBLEM_SHARK_H_
