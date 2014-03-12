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

// TODO: make attribute limits parameters

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class HoeffdingTreeLearnerExperiments : public WorkUnit
{
public:
  HoeffdingTreeLearnerExperiments() : numSamples(50), randomSeed(0), learningRate(0.1), learningRateDecay(0.005) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    

    // set up test problems
    std::vector<ProblemPtr> problems;
    problems.push_back(new DTLZ1MOProblem(1, 1));
    problems.push_back(new DTLZ2MOProblem(1, 1));
    problems.push_back(new DTLZ3MOProblem(1, 1));
    problems.push_back(new DTLZ4MOProblem(1, 1));
    problems.push_back(new DTLZ5MOProblem(1, 1));
    problems.push_back(new DTLZ6MOProblem(1, 1));
    problems.push_back(new DTLZ7MOProblem(1, 1));

    // create the domain
//    for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
//    {
    size_t functionNumber = 0;
      ExpressionDomainPtr domain = new ExpressionDomain();
      domain->addInput(doubleClass, "x");
      domain->createSupervision(doubleClass, "y");
    
      // create the learning problem
      ProblemPtr baseProblem = problems[functionNumber];
      SamplerPtr sampler = uniformSampler();
      sampler->initialize(context, baseProblem->getDomain());
      ProblemPtr problem = baseProblem->toSupervisedLearningProblem(context, numSamples, 100, sampler);
    
      // dit veranderen van perceptronIncrementalLearner naar hoeffdingTreeLearner()
      //SolverPtr learner = incrementalLearnerBasedLearner(perceptronIncrementalLearner(30, learningRate, learningRateDecay));
      SolverPtr learner = incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(context, randomSeed, NXY, Hoeffding, 0.01));
    
      ObjectivePtr problemObj = problem->getObjective(0);
      const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    
      TablePtr testTable = makeTestTable(domain->getInput(0));

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
      VectorPtr predictions = model->compute(context, testTable)->getVector();
      context.resultCallback("predictions", predictions);
      double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
      context.resultCallback("testingScore", testingScore);
      makeCurve(context, baseProblem, model);
      context.leaveScope();
 //   }
    return new Boolean(true);
  }
  
protected:
  friend class HoeffdingTreeLearnerExperimentsClass;
  
  size_t numSamples;
  int randomSeed;
  double learningRate;
  double learningRateDecay;

private:

  void makeCurve(ExecutionContext& context, ProblemPtr baseProblem, ExpressionPtr expression)
  {
      //context.enterScope("Curve");
      double x = 0.0;
      size_t curveSize = 100;
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
    }
    jassert(false);
    return 0.0;
  }
};

}; /* namespace lbcpp */

#endif // MOO_HOEFFDING_TREE_LEARNER_EXPERIMENTS_H_
