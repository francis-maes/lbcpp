/*-------------------------------------------.-----------------------------------------.
 | Filename: PerceptronTest.h                | Testing Perceptron Code                 |
 | Author  : Denny Verbeeck                  |                                         |
 | Started : 10/12/2013 15:51                |                                         |
 `-------------------------------------------/                                         |
                                  |                                                    |
                                  `---------------------------------------------------*/

#ifndef MOO_PERCEPTRON_TEST_H_
# define MOO_PERCEPTRON_TEST_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class PerceptronTest : public WorkUnit
{
public:
  PerceptronTest() : numSamples(25), randomSeed(0) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    
    // create the domain
    for (size_t functionNumber = 0; functionNumber < 9; ++functionNumber)
    {
      ExpressionDomainPtr domain = new ExpressionDomain();
      VariableExpressionPtr x = domain->addInput(doubleClass, "x");
      VariableExpressionPtr y = domain->createSupervision(doubleClass, "y");
    
      // create the learning problem
      ProblemPtr problem = makeProblem(context, functionNumber, domain);
    
      // put learners in a vector
      SolverPtr learner = incrementalLearnerBasedLearner(perceptronIncrementalLearner(30, 0.1, 0.005));
    
      ObjectivePtr problemObj = problem->getObjective(0);
      const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    
      TablePtr testTable = makeTestTable(domain->getInput(0));

      ExpressionPtr model;
      FitnessPtr fitness;
      context.enterScope("Function " + ((string) functionNumber));
      context.enterScope("Learning");
      learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
      context.leaveScope();
      context.resultCallback("model", model);
      context.resultCallback("fitness", fitness);      
      context.resultCallback("data", problemData);
      VectorPtr predictions = model->compute(context, testTable)->getVector();
      context.resultCallback("predictions", predictions);
      double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
      context.resultCallback("testingScore", testingScore);
      makeCurve(context, functionNumber, model);
      context.leaveScope();
    }

    /*
    for (size_t i = 0; i < solvers.size(); i++)
    {
      // learn
      SolverPtr learner = solvers[i];
      context.enterScope(learner->toShortString());
      ExpressionPtr model;
      FitnessPtr fitness;
      context.enterScope("Learning");
      learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
      context.leaveScope();
      context.resultCallback("model", model);
      context.resultCallback("fitness", fitness);      
      context.resultCallback("data", problemData);
      // evaluate
      double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
      context.resultCallback("testingScore", testingScore);
      
      // make curve
      context.enterScope("test");
      
      VectorPtr predictions = model->compute(context, testTable)->getVector();
      context.resultCallback("predictions", predictions);
      
      // find best fitness
      FitnessPtr bestFitness;
      FitnessLimitsPtr limits = new FitnessLimits();
      double worst, best;
      problem->getObjective(0)->getObjectiveRange(worst, best);
      limits->addObjective(worst, best);
      for (size_t r = 0; r < problemData->getNumRows(); r++)
      {
        ObjectPtr supervision = problemData->getElement(r, 1);
        FitnessPtr fitnessR = new Fitness(Double::get(supervision), limits);
        bestFitness = (!bestFitness || fitnessR->dominates(bestFitness) ? fitnessR : bestFitness);
      }
      
      SelectionCriterionPtr greedy = greedySelectionCriterion();
      greedy->initialize(problem);
      SelectionCriterionPtr optimistic = optimisticSelectionCriterion(2.0);
      optimistic->initialize(problem);
      SelectionCriterionPtr probabilityOfImprovement = probabilityOfImprovementSelectionCriterion(bestFitness);
      probabilityOfImprovement->initialize(problem);
      SelectionCriterionPtr expectedImprovement = expectedImprovementSelectionCriterion(bestFitness);
      expectedImprovement->initialize(problem);
      
      double x = -1.0;
      for (size_t i = 0; i < 200; ++i, x += 0.01)
      {
        context.enterScope(string(x));
        context.resultCallback("x", x);
       
        for (size_t j = 0; j < numObjectives; ++j)
        {
          context.resultCallback("supervision" + string((int)j), targetFunction(x, moFunctionIdx[j]));
        
          bool found = false;
          for (size_t r = 0; r < problemData->getNumRows() && !found; r++)
          {
            if (fabs(Double::get(problemData->getElement(r,0)) - x) < 0.005)
            {
              found = true;
              ObjectPtr supervision = problemData->getElement(r, 1);
              context.resultCallback("examples" + string((int)j), getIthValue(supervision, j));
            }
          }
          
          ScalarVariableMeanAndVariancePtr test = predictions->getElement(0).dynamicCast<ScalarVariableMeanAndVariance>();
          if (test)  // it's an ensemble
          {
            ScalarVariableMeanAndVariancePtr prediction;
            if (numObjectives == 1)
              prediction = predictions->getElement(i).staticCast<ScalarVariableMeanAndVariance>();
            else
            {
              OVectorPtr multiPrediction = predictions->getElement(i).staticCast<OVector>();
              prediction = multiPrediction->getAndCast<ScalarVariableMeanAndVariance>(j);
            }
            double pred = prediction->getMean();
            double stddev = prediction->getStandardDeviation();
            context.resultCallback("prediction" + string((int)j), pred);
            //context.resultCallback("stddevUp" + string((int)j), pred + stddev);
            //context.resultCallback("stddevDown" + string((int)j), pred - stddev);
            context.resultCallback("stddev" + string((int)j), stddev);
            if (numObjectives == 1)
            {
              if (fabs(x - 0.2) < 1e-6)
                jassert(true);
              context.resultCallback("greedySelectionCriterion", greedy->evaluate(context, prediction));
              context.resultCallback("optimisticSelectionCriterion", optimistic->evaluate(context, prediction));
              context.resultCallback("probabilityOfImprovementSelectionCriterion", probabilityOfImprovement->evaluate(context, prediction));
              context.resultCallback("expectedImprovementSelectionCriterion", expectedImprovement->evaluate(context, prediction));
            }

          }
          else
          {
            context.resultCallback("prediction" + string((int)j), getIthValue(predictions->getElement(i), j));
          }
        }
        context.leaveScope();
      }
      context.leaveScope();
      context.leaveScope();
    }*/
    
    return new Boolean(true);
  }
  
protected:
  friend class PerceptronTestClass;
  
  size_t numSamples;
  int randomSeed;

private:

  void makeCurve(ExecutionContext& context, size_t functionNumber, ExpressionPtr expression)
  {
      context.enterScope("Curve");
      double x = -1.0;
      std::vector<ObjectPtr> input = std::vector<ObjectPtr>(1);
      input[0] = new Double(0.0);
      for (size_t i = 0; i < 200; ++i, x += 0.01)
      {
        context.enterScope(string(x));
        context.resultCallback("x", x);
        context.resultCallback("supervision", targetFunction(context, x, functionNumber));
        input[0].staticCast<Double>()->set(x);
        context.resultCallback("prediction", expression->compute(context, input));
        context.leaveScope();
      }
      context.leaveScope();
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
      double x = random->sampleDouble(-1.0,1.0);
      x = juce::roundDoubleToInt(x * 100) / 100.0;
      res->setElement(i, 0, new Double(x));
      
      double y = targetFunction(context, x, functionNumber);
      res->setElement(i, 1, new Double(y));
    }
    return res;
  }
  
  TablePtr makeTestTable(VariableExpressionPtr xVariable) const
  {
    TablePtr res = new Table(200);
    res->addColumn(xVariable, xVariable->getType());
    double x = -1.0;
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
    }
    jassert(false);
    return 0.0;
  }
};

}; /* namespace lbcpp */

#endif // MOO_PERCEPTRON_TEST_H_
