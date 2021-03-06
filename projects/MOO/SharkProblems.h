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
# include <ml/Expression.h>
# include <ml/ExpressionDomain.h>
# include <EALib/ObjectiveFunctions.h>
# include <EALib/MultiObjectiveFunctions.h>
# include <ml/DoubleVector.h>

#define round(x) (floor((x) + 0.5))

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
  ProblemFromSharkObjectiveFunction(ObjectiveFunctionVS<double>* objective, bool randomizeOptimum = true) 
    : numDimensions(objective->dimension()), objective(objective), randomizeOptimum(randomizeOptimum) {}
  ProblemFromSharkObjectiveFunction(size_t numDimensions, ObjectiveFunctionVS<double>* objective, bool randomizeOptimum = true)
    : numDimensions(numDimensions), objective(objective), randomizeOptimum(randomizeOptimum) {}

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
    DenseDoubleVectorPtr optimum;
    if (randomizeOptimum)
      optimum = scalarVectorDomain->sampleUniformly(context.getRandomGenerator()).staticCast<DenseDoubleVector>();
    else
      optimum = new DenseDoubleVector(scalarVectorDomain->getNumDimensions(), 0.0);

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
  bool randomizeOptimum;
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
    : ProblemFromSharkObjectiveFunction(objective, false), max1(max1), max2(max2)
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = objectiveIndex == 1 ? max2 : max1; best = 0.0;}

private:
  double max1;
  double max2;
};

struct ZDT1MOProblem : public ZDTMOProblem {ZDT1MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT1(numDimensions), 1.0, 1.0) {} };
struct ZDT2MOProblem : public ZDTMOProblem {ZDT2MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT2(numDimensions), 1.0, 1.0) {} };
struct ZDT3MOProblem : public ZDTMOProblem {ZDT3MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT3(numDimensions), 0.852, 1.0) {} };
struct ZDT4MOProblem : public ZDTMOProblem {ZDT4MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT4(numDimensions), 1.0, 1.0) {} };
// ZDT5 not wrapper yet (domain is discrete)
struct ZDT6MOProblem : public ZDTMOProblem {ZDT6MOProblem(size_t numDimensions = 30) : ZDTMOProblem(new ZDT6(numDimensions), 1.0, 1.0) {} };
  

/*
** LZ06 functions
*/
  
class LZ06MOProblem : public ProblemFromSharkObjectiveFunction
{
public:
  LZ06MOProblem(ObjectiveFunctionVS<double>* objective, double max1, double max2)
    : ProblemFromSharkObjectiveFunction(objective, false), max1(max1), max2(max2)
    {initialize(defaultExecutionContext());}
  
  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
    {worst = objectiveIndex == 1 ? max2 : max1; best = 0.0;}
  
private:
  double max1;
  double max2;
};
  
struct LZ06_F1MOProblem : public LZ06MOProblem {LZ06_F1MOProblem(size_t numDimensions = 30) : LZ06MOProblem(new LZ06_F1(numDimensions), 1.0, 1.0) {} };

/*
** DTLZ functions
*/

class DTLZMOProblem : public ProblemFromSharkObjectiveFunction
{
public:
  DTLZMOProblem(ObjectiveFunctionVS<double>* objective, double max1, double max2, double max3) 
    : ProblemFromSharkObjectiveFunction(objective, false), max1(max1), max2(max2), max3(max3)
    {initialize(defaultExecutionContext());}

  virtual void getObjectiveRange(size_t objectiveIndex, double& worst, double& best) const
  {
    best = 0.0;
    switch(objectiveIndex)
    {
    case 0:
      worst = max1;
      break;
    case 1:
      worst = max2;
      break;
    case 2:
      worst = max3;
      break;
    default:
      jassertfalse;
      break;
    }
  }

private:
  double max1;
  double max2;
  double max3;
};

struct DTLZ1MOProblem : public DTLZMOProblem {DTLZ1MOProblem(size_t numDimensions =  7, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ1(numDimensions, numObjectives), 0.5, 0.5, 0.5) {} };
struct DTLZ2MOProblem : public DTLZMOProblem {DTLZ2MOProblem(size_t numDimensions = 12, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ2(numDimensions, numObjectives), 1.0, 1.0, 1.0) {} };
struct DTLZ3MOProblem : public DTLZMOProblem {DTLZ3MOProblem(size_t numDimensions = 12, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ3(numDimensions, numObjectives), 1.0, 1.0, 1.0) {} };
struct DTLZ4MOProblem : public DTLZMOProblem {DTLZ4MOProblem(size_t numDimensions = 12, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ4(numDimensions, numObjectives), 1.0, 1.0, 1.0) {} };
struct DTLZ5MOProblem : public DTLZMOProblem {DTLZ5MOProblem(size_t numDimensions = 12, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ5(numDimensions, numObjectives), 1.0, 1.0, 1.0) {} };
struct DTLZ6MOProblem : public DTLZMOProblem {DTLZ6MOProblem(size_t numDimensions = 12, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ6(numDimensions, numObjectives), 1.0, 1.0, 1.0) {} };
struct DTLZ7MOProblem : public DTLZMOProblem {DTLZ7MOProblem(size_t numDimensions = 22, size_t numObjectives = 2) : DTLZMOProblem(new DTLZ7(numDimensions, numObjectives), 2.12, 4.0, 6.0) {} };


class FriedmannObjective : public Objective
{
public:
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr input = object.staticCast<DenseDoubleVector>();
    return 10.0 * sin(M_PI * input->getValue(0) * input->getValue(1)) + 20.0 * (input->getValue(2) - 0.5) * (input->getValue(2) - 0.5) +
      10.0 * input->getValue(3) + 5.0 * input->getValue(4) + context.getRandomGenerator()->sampleDoubleFromGaussian(0.0, 1.0);
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = -20.0; best = 40;}
};

class FriedmannProblem : public Problem
{
public:
  FriedmannProblem()
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(10, std::make_pair(0, 1.0))));
    addObjective(new FriedmannObjective());
  }
};

class LEXPProblem : public Problem
{
public:
  LEXPProblem()
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(5, std::make_pair(0, 1.0))));
    addObjective(new LEXPObjective());
  }

protected:
  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(5, std::make_pair(0, 1.0))));
    addObjective(new LEXPObjective());
  }

private:
  class LEXPObjective : public Objective
  {
  public:
    LEXPObjective() {};

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
	    double x1 = object.staticCast<DenseDoubleVector>()->getValue(0);
      double x2 = object.staticCast<DenseDoubleVector>()->getValue(1);
      double x3 = object.staticCast<DenseDoubleVector>()->getValue(2);
      double x4 = object.staticCast<DenseDoubleVector>()->getValue(3);
      double x5 = object.staticCast<DenseDoubleVector>()->getValue(4);
      return round(x1)*(1+2*x2+3*x3-exp(-2*(x4+x5)))+(1-round(x1))*(1-1.2*x2-3.1*x3+exp(-3*(x4-x5)));
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = -3.25; best = 5.98;}
  };
};

class LOSCProblem : public Problem
{
public:
  LOSCProblem()
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(5, std::make_pair(0, 1.0))));
    addObjective(new LOSCObjective());
  }

protected:
  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(5, std::make_pair(0, 1.0))));
    addObjective(new LOSCObjective());
  }

private:
  class LOSCObjective : public Objective
  {
  public:
    LOSCObjective() {};

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
	    double x1 = object.staticCast<DenseDoubleVector>()->getValue(0);
      double x2 = object.staticCast<DenseDoubleVector>()->getValue(1);
      double x3 = object.staticCast<DenseDoubleVector>()->getValue(2);
      double x4 = object.staticCast<DenseDoubleVector>()->getValue(3);
      double x5 = object.staticCast<DenseDoubleVector>()->getValue(4);
      return round(x1)*(1+1.5*x2+x3+sin(2*(x4+x5))*exp(-2*(x2+x4)))+(1-round(x1))*(-1-2*x2-x3+sin(3*(x4+x5))*exp(-3*(x3-x4)));
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0; best = 1;}
  };
};

class ParaboloidProblem : public Problem
{
public:
  ParaboloidProblem()
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(2, std::make_pair(-4.0, 4.0))));
    addObjective(new ParaboloidObjective());
  }

protected:
  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(2, std::make_pair(-4.0, 4.0))));
    addObjective(new ParaboloidObjective());
  }

private:
  class ParaboloidObjective : public Objective
  {
  public:
    ParaboloidObjective() {}

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
      double x1 = object.staticCast<DenseDoubleVector>()->getValue(0);
      double x2 = object.staticCast<DenseDoubleVector>()->getValue(1);
      return sqrt(x1 * x1 + x2 * x2);
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = 0.0; best = 5.66;}
  };
};

extern ProblemPtr paraboloidProblem();

class CARTProblem : public Problem
{
public:
  CARTProblem()
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(10, std::make_pair(-1.0, 1.0))));
    addObjective(new CARTObjective());
  }

  virtual ProblemPtr toSupervisedLearningProblem(ExecutionContext& context, size_t numSamples, size_t numValidationSamples, SamplerPtr sampler) const
  {
    ExpressionDomainPtr domain = new ExpressionDomain();
    TablePtr supervision = new Table(numSamples);
    TablePtr validation = new Table(numValidationSamples);
    
    for (size_t i = 0; i < getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions(); ++i)
    {
      VariableExpressionPtr x = domain->addInput(doubleClass, "x" + string((int) i));
      supervision->addColumn(x, doubleClass);
      validation->addColumn(x, doubleClass);
    }
    VariableExpressionPtr y;
    if (this->getNumObjectives() > 1)
      {jassertfalse;}
    else
      y = domain->createSupervision(doubleClass, "y");
    supervision->addColumn(y, y->getType());
    validation->addColumn(y, y->getType());

    // fill the tables
    DenseDoubleVectorPtr sample = new DenseDoubleVector(10, 0.0);
    for (size_t i = 0; i < numSamples; ++i)
    {
      sample->setValue(0, context.getRandomGenerator()->sampleBool() ? -1.0 : 1.0);
      for (size_t j = 1; j < 10; ++j)
        sample->setValue(j, context.getRandomGenerator()->sampleInt(-1, 2));
      FitnessPtr result = evaluate(context, sample);
      for (size_t j = 0; j < sample->getNumValues(); ++j)
        supervision->setElement(i, j, new Double(sample->getValue(j)));
      supervision->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
    }
    for (size_t i = 0; i < numValidationSamples; ++i)
    {
      sample->setValue(0, context.getRandomGenerator()->sampleBool() ? -1.0 : 1.0);
      for (size_t j = 1; j < 10; ++j)
        sample->setValue(j, context.getRandomGenerator()->sampleInt(-1, 2));
      FitnessPtr result = evaluate(context, sample);
      for (size_t j = 0; j < sample->getNumValues(); ++j)
        validation->setElement(i, j, new Double(sample->getValue(j)));
      validation->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
    }

    ProblemPtr res = new Problem();
    res->setThisClass(getClass());
    
    res->setDomain(domain);
    res->addObjective(rmseRegressionObjective(supervision, y));
    res->addValidationObjective(rrseRegressionObjective(supervision, validation, y));

    return res;
  }

  virtual std::vector<ProblemPtr> generateFolds(ExecutionContext& context, size_t numFolds, size_t samplesPerFold, SamplerPtr sampler) const
  {
    std::vector<ProblemPtr> res(numFolds);
    std::vector<VariableExpressionPtr> variables;
    size_t numSamples = numFolds * samplesPerFold;
    TablePtr samples = new Table(numSamples);
    ExpressionDomainPtr domain = new ExpressionDomain();

    for (size_t i = 0; i < getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions(); ++i)
    {
      VariableExpressionPtr x = domain->addInput(doubleClass, "x" + string((int) i));
      variables.push_back(x);
      samples->addColumn(x, doubleClass);
    }
    VariableExpressionPtr y;
    if (this->getNumObjectives() > 1)
      {jassertfalse;}
    else
      y = domain->createSupervision(doubleClass, "y");
    samples->addColumn(y, y->getType());

    DenseDoubleVectorPtr sample = new DenseDoubleVector(10, 0.0);
    for (size_t i = 0; i < numSamples; ++i)
    {
      sample->setValue(0, context.getRandomGenerator()->sampleBool() ? -1.0 : 1.0);
      for (size_t j = 1; j < 10; ++j)
        sample->setValue(j, context.getRandomGenerator()->sampleInt(-1, 2));
      FitnessPtr result = evaluate(context, sample);
      for (size_t j = 0; j < sample->getNumValues(); ++j)
        samples->setElement(i, j, new Double(sample->getValue(j)));
      samples->setElement(i, sample->getNumValues(), new Double(result->getValue(0)));
    }

    for (size_t i = 0; i < numFolds; ++i)
    {
      TablePtr foldTrain = new Table();
      TablePtr foldTest = new Table();
      for (size_t j = 0; j < variables.size(); ++j)
      {
        foldTrain->addColumn(variables[j], variables[j]->getType());
        foldTest->addColumn(variables[j], variables[j]->getType());
      }
      foldTrain->addColumn(y, y->getType());
      foldTest->addColumn(y, y->getType());
      for (size_t j = 0; j < numSamples; ++j)
      {
        if (((int)j - (int)i) % (int)numFolds == 0)
          foldTest->addRow(samples->getRow(j));
        else
          foldTrain->addRow(samples->getRow(j));
      }
      ProblemPtr fold = new Problem();
      fold->setDomain(domain);
      fold->addObjective(rmseRegressionObjective(foldTrain, y));
      fold->addValidationObjective(rrseRegressionObjective(foldTrain, foldTest, y));
      res[i] = fold;
    }

    return res;
  }

protected:
  virtual void initialize(ExecutionContext& context)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(10, std::make_pair(-1.0, 1.0))));
    addObjective(new CARTObjective());
  }

private:
  class CARTObjective : public Objective
  {
  public:
    CARTObjective() {}

    virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
    {
      DenseDoubleVectorPtr s = object.staticCast<DenseDoubleVector>();
      if (s->getValue(0) < 0.0)
        return 3 + 3 * s->getValue(1) + 2 * s->getValue(2) + 3 * s->getValue(3);
      else
        return -3 + 3 * s->getValue(4) + 2 * s->getValue(5) + s->getValue(6);
    }

    virtual void getObjectiveRange(double& worst, double& best) const
      {worst = -9.0; best = 9.0;}
  };
};


}; /* namespace lbcpp */

#endif // !MOO_PROBLEM_SHARK_H_
