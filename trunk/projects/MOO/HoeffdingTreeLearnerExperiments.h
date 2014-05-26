/*---------------------------------------------.-----------------------------------------.
 | Filename: HoeffdingTreeLearnerExperiments.h | Experiments for the Hoeffding Tree      |
 | Author  : Denny Verbeeck                    | Learner                                 |
 | Started : 10/12/2013 15:51                  |                                         |
 `---------------------------------------------/                                         |
                                  |                                                      |
                                  `-----------------------------------------------------*/

#ifndef MOO_HOEFFDING_TREE_LEARNER_EXPERIMENTS_H_
# define MOO_HOEFFDING_TREE_LEARNER_EXPERIMENTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>
# include "SharkProblems.h"
# include "../../lib/ml/Learner/IncrementalLearnerBasedLearner.h"

// TODO: make attribute limits parameters

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class OneDimFunctionObjective : public Objective
{
public:
  OneDimFunctionObjective(size_t functionIndex) : functionIndex(functionIndex) {};

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
	double x = object.staticCast<DenseDoubleVector>()->getValue(0);
	switch (functionIndex)
    {
      case 0: return x < 0.5 ? 2 * x : -2 * x + 2;
      case 1: 
		  if(x < 0.25)
		    return 4 * x;
		  else if(x < 0.5)
		    return -4 * x + 2;
		  else if(x < 0.75)
			return 4 * x - 2;
		  else
			return -4 * x + 4;
	  default: return 0;
	}
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0; best = 1;}

protected:
  size_t functionIndex;
};

class TwoDimFunctionObjective : public Objective
{
public:
  TwoDimFunctionObjective(size_t functionIndex) : functionIndex(functionIndex) {};

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
	double x1 = object.staticCast<DenseDoubleVector>()->getValue(0);
  double x2 = object.staticCast<DenseDoubleVector>()->getValue(1);
	switch (functionIndex)
    {
  case 0: return max(exp(-10*x1*x1),max(exp(-50*x2*x2),1.25*exp(-5*(x1*x1+x2*x2))));
	  default: return 0;
	}
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = -1; best = 1;}

protected:
  size_t functionIndex;
};

class OneDimFunctionProblem : public Problem
{
public:
  OneDimFunctionProblem(size_t functionIndex)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(1, std::make_pair(0, 1.0))));
    addObjective(new OneDimFunctionObjective(functionIndex));
  }
};

class TwoDimFunctionProblem : public Problem
{
public:
  TwoDimFunctionProblem(size_t functionIndex)
  {
    setDomain(new ScalarVectorDomain(std::vector< std::pair<double, double> >(2, std::make_pair(-1.0, 1.0))));
    addObjective(new TwoDimFunctionObjective(functionIndex));
  }
};
  
class HoeffdingTreeLearnerExperiments : public WorkUnit
{
public:
  HoeffdingTreeLearnerExperiments() : numSamples(50), randomSeed(0), learningRate(2.0), learningRateDecay(0.05), delta(0.2), threshold(0.15), numDims(1), verbosity(verbosityQuiet) {}//verbosityDetailed
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);

    // set up test problems
    std::vector<ProblemPtr> problems;
    /*problems.push_back(new DTLZ1MOProblem(1, 1));
    problems.push_back(new DTLZ2MOProblem(2, 1));
    problems.push_back(new DTLZ3MOProblem(1, 1));
    problems.push_back(new DTLZ4MOProblem(1, 1));
    problems.push_back(new DTLZ5MOProblem(1, 1));
    problems.push_back(new DTLZ6MOProblem(1, 1));
    problems.push_back(new DTLZ7MOProblem(1, 1));
    //problems.push_back(new FriedmannProblem());
	  problems.push_back(new OneDimFunctionProblem(0));*/
	  problems.push_back(new OneDimFunctionProblem(1));
    problems.push_back(new TwoDimFunctionProblem(0));
    //problems.push_back(new LEXPProblem());
    //problems.push_back(new LOSCProblem());

    // grid search for optimal parameters
    gridSearch(context, problems);
    
    SamplerPtr sampler = uniformSampler();

    for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
    {
      // create the learning problem
      ProblemPtr baseProblem = problems[functionNumber];
      sampler->initialize(context, baseProblem->getDomain());
      ProblemPtr problem = baseProblem->toSupervisedLearningProblem(context, numSamples, 100, sampler);
    
      // dit veranderen van perceptronIncrementalLearner naar hoeffdingTreeLearner()
      //SolverPtr learner = incrementalLearnerBasedLearner(perceptronIncrementalLearner(30, learningRate, learningRateDecay));
	  // mauveIncrementalSplittingCriterion(0.01, 0.4, 0.95)
	  // quandtAndrewsIncrementalSplittingCriterion(2, 0.001)
      SolverPtr learner = incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(mauveIncrementalSplittingCriterion(0.01, 0.4, 0.95), simpleLinearRegressionIncrementalLearner(), 50));
      //SolverPtr learner = incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(quandtAndrewsIncrementalSplittingCriterion(2, 0.001), simpleLinearRegressionIncrementalLearner(), 50));
      //SolverPtr learner = incrementalLearnerBasedLearner(simpleLinearRegressionIncrementalLearner());
      learner->setVerbosity((SolverVerbosity)verbosity);
      learner.staticCast<IncrementalLearnerBasedLearner>()->baseProblem = baseProblem;
    
      ObjectivePtr problemObj = problem->getObjective(0);
      const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    
      ExpressionPtr model;
      FitnessPtr fitness;
      context.enterScope("Function " + string((int) functionNumber));
      context.enterScope("Learning");
      context.resultCallback("x", 0.0);
      learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
      context.leaveScope();
      context.resultCallback("model", model);
      context.resultCallback("fitness", fitness);      
      context.resultCallback("data", problemData);
      double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
      context.resultCallback("testingScore", testingScore);
      context.resultCallback("tree size", model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves());
      makeCurve(context, baseProblem, model);
      //makeMatlabSurface(context, baseProblem, model);
      context.leaveScope(testingScore);
    }
    return new Boolean(true);
  }
  
protected:
  friend class HoeffdingTreeLearnerExperimentsClass;
  
  size_t numSamples;
  int randomSeed;
  double learningRate;
  double learningRateDecay;
  double delta;
  double threshold;
  size_t numDims;
  size_t verbosity;

private:
    void makeMatlabSurface(ExecutionContext& context, ProblemPtr problem, ExpressionPtr expr)
  {
    if (problem->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() <= 1) return;
    std::ofstream file, file2, file3;
    file.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\surface.dat");
    file2.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\realsurface.dat");
    file3.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\error.dat");
    std::vector<ObjectPtr> point(2);
    DenseDoubleVectorPtr vpoint = new DenseDoubleVector(2, 0.0);
    for (size_t i = 0; i < 100; ++i)
    {
      point[1] = new Double(i / 100.0);
      vpoint->setValue(1, i / 100.0);
      for (size_t j = 0; j < 100; ++j)
      {
        point[0] = new Double(j / 100.0);
        vpoint->setValue(0, j / 100.0);
        ObjectPtr result = expr->compute(context, point);
        FitnessPtr realval = problem->evaluate(context, vpoint);
        file << Double::get(result) << " ";
        file2 << realval->getValue(0) << " ";
        file3 << abs(Double::get(result) - realval->getValue(0)) << " ";
      }
      file << std::endl;
      file2 << std::endl;
      file3 << std::endl;
    }
    file.close();
    file2.close();
    file3.close();
  }

  void makeCurve(ExecutionContext& context, ProblemPtr baseProblem, ExpressionPtr expression)
  {
      //context.enterScope("Curve");
    if (baseProblem->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() > 1) return;
    double x = 0.0;
    size_t curveSize = 200;
    std::vector<ObjectPtr> input = std::vector<ObjectPtr>(1);
    input[0] = new Double(0.0);
    DenseDoubleVectorPtr problemInput = new DenseDoubleVector(1, 0.0);
    ScalarVectorDomainPtr domain = baseProblem->getDomain().staticCast<ScalarVectorDomain>();
    double range = domain->getUpperLimit(0) - domain->getLowerLimit(0);
    double offset = domain->getLowerLimit(0);
    for (size_t i = 0; i < curveSize; ++i)
    {
      x = offset + range * i / curveSize;
      context.enterScope(string(x));
      context.resultCallback("x", x);
      input[0].staticCast<Double>()->set(x);
      problemInput->setValue(0, x);
      context.resultCallback("supervision", baseProblem->evaluate(context, problemInput)->getValue(0));
      context.resultCallback("prediction", expression->compute(context, input));
      context.leaveScope();
    }
    //context.leaveScope();
  }

  void gridSearch(ExecutionContext& context, std::vector<ProblemPtr> problems)
  {
    SamplerPtr sampler = uniformSampler();
    SolverPtr learner;
    double steps = 50;
    std::vector< double > vec_t(steps);
    std::vector< std::vector< double > > vec_dt( steps, vec_t );
    std::vector< std::vector< std::vector< double > > > vec_pdt(problems.size(), vec_dt );

    for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
    {
      // create the learning problem
      ProblemPtr baseProblem = problems[functionNumber];
      sampler->initialize(context, baseProblem->getDomain());
      ProblemPtr problem = baseProblem->toSupervisedLearningProblem(context, numSamples, 100, sampler);
      int i = 0;
      int di = -1;
      for(double d = 0.01; d <= 0.50001; d+=0.01)
      {
        di++;
        int ti = -1;
        for(double t=0.01; t <= 0.50001; t+=0.01)
        {
          ti++;
          i++;
          learner = incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(mauveIncrementalSplittingCriterion(d, t, 0.95), simpleLinearRegressionIncrementalLearner(), 10));
          learner->setVerbosity((SolverVerbosity)verbosity);
          learner.staticCast<IncrementalLearnerBasedLearner>()->baseProblem = baseProblem;
          ExpressionPtr model;
          FitnessPtr fitness;
          learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
          double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
          std::cout << d << " " << t << " " << i << " " << testingScore << std::endl;
          vec_pdt[functionNumber][di][ti] = testingScore;
        }
      }
    }
    // write the results for matlab
    std::ofstream file;
    file.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\gridsearch.dat");
    for(double di=0; di < steps; di++)
    {
      for(double ti=0; ti < steps; ti++)
      {
        double sum = 0;
        for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
        {
          sum += vec_pdt[functionNumber][di][ti];
        }
        sum /= problems.size();
        file << sum << " ";
      }
      file << std::endl;
    }
      file.close();
  }
      
  ProblemPtr makeProblem(ExecutionContext& context, size_t functionNumber, ExpressionDomainPtr domain)
  {
    ProblemPtr res = new Problem();
    
    VariableExpressionPtr x = domain->getInput(0);
    VariableExpressionPtr y = domain->getSupervision();
    
    res->setDomain(domain);
    res->addObjective(mseRegressionObjective(makeTable(context, functionNumber, numSamples, x, y), y));
    res->addValidationObjective(mseRegressionObjective(makeTable(context, functionNumber, 101, x, y), y));

    return res;
  }
  
  TablePtr makeTable(ExecutionContext& context, size_t functionNumber, size_t count, VariableExpressionPtr x, VariableExpressionPtr y)
  {
    TablePtr res = new Table(count);
    res->addColumn(x, doubleClass);
    res->addColumn(y, y->getType());
    RandomGeneratorPtr random = context.getRandomGenerator();
    for (size_t i = 0; i < count; ++i)
    {
      double x = random->sampleDouble(0.0,1.0);
      x = juce::roundDoubleToInt(x * 100) / 100.0;
      res->setElement(i, 0, new Double(x));
      
      double y = targetFunction(context, x, functionNumber);
      res->setElement(i, 1, new Double(y));
    }
    return res;
  }
  
  TablePtr makeTestTable(VariableExpressionPtr xVariable) const
  {
    TablePtr res = new Table(100);
    res->addColumn(xVariable, xVariable->getType());
    double x = 0.0;
    for (size_t i = 0; i < res->getNumRows(); ++i)
    {
      res->setElement(i, 0, new Double(x));
      x += 0.01;
      x = juce::roundDoubleToInt(x * 100) / 100.0;
    }
    return res;
  }
  
  double targetFunction(ExecutionContext& context, double x, size_t functionIndex)
  {
    double x2 = x * x;
    switch (functionIndex)
    {
      case 0: return sin(x2) * cos(x) + 1.0;
      case 1: return x * x2 + x2 + x;
      case 2: return x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
      case 3: return x2 * x2 * x2 + x * x2 * x2 + x2 * x2 + x * x2 + x2 + x;
      case 4: return x2 * x2 + x * x2 + x2 + x;
      case 5: return sin(x) + sin(x + x2);
      case 6: return log(x + 2) + log(x2 + 1);
      case 7: return sqrt(2 + x);
      case 8: return sqrt(2.0) * x - 1.0 + context.getRandomGenerator()->sampleDoubleFromGaussian(0.0, 0.1);
      case 9: 
        if (x < 0.25) return x;
        else if (x < 0.5) return -x + 0.5;
        else if (x < 0.75) return x - 0.5;
        else return -x + 1;
      case 10: return x;
      case 11:
        if (x < 0.5) return x;
        else return 1.0 - x;
    }
    jassert(false);
    return 0.0;
  }
};

}; /* namespace lbcpp */

#endif // MOO_HOEFFDING_TREE_LEARNER_EXPERIMENTS_H_
