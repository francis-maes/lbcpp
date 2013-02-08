/*-------------------------------------------.-----------------------------------------.
 | Filename: TestRandomForestUncertainties.h | Testing Uncertainties of Random Forests |
 | Author  : Denny Verbeeck                  |                                         |
 | Started : 27/11/2012 11:30                |                                         |
 `-------------------------------------------/                                         |
 |                                                                                     |
 `------------------------------------------------------------------------------------*/

#ifndef MOO_TEST_RANDOM_FOREST_UNCERTAINTIES_H_
# define MOO_TEST_RANDOM_FOREST_UNCERTAINTIES_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>

namespace lbcpp
{
  
  extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

  class TestRandomForestUncertainties : public WorkUnit
  {
  public:
    TestRandomForestUncertainties() : numSamples(25), numTrees(100) {}
    
    virtual ObjectPtr run(ExecutionContext& context)
    {
      lbCppMLLibraryCacheTypes(context);

      ProblemPtr problem = makeProblem(context);
      
      // make DT learner
      SamplerPtr DTexpressionVectorSampler = scalarExpressionVectorSampler();
      SolverPtr DTconditionLearner = exhaustiveConditionLearner(DTexpressionVectorSampler);
      
      SolverPtr DTlearner = treeLearner(stddevReductionSplittingCriterion(), DTconditionLearner);
      DTlearner->setVerbosity(verbosityDetailed);

      // make RF learner
      SamplerPtr RFexpressionVectorSampler = scalarExpressionVectorSampler();
      SolverPtr RFconditionLearner = exhaustiveConditionLearner(RFexpressionVectorSampler);
      //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr RFlearner = treeLearner(stddevReductionSplittingCriterion(), RFconditionLearner); 
      //learner->setVerbosity((SolverVerbosity)verbosity);
      RFlearner = baggingLearner(RFlearner, numTrees);
      RFlearner->setVerbosity(verbosityDetailed);
      
      // make RF learner
      SamplerPtr RF2expressionVectorSampler = scalarExpressionVectorSampler();
      SolverPtr RF2conditionLearner = randomSplitConditionLearner(RF2expressionVectorSampler);
      //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr RF2learner = treeLearner(stddevReductionSplittingCriterion(), RF2conditionLearner); 
      //learner->setVerbosity((SolverVerbosity)verbosity);
      RF2learner = baggingLearner(RF2learner, numTrees);
      RF2learner->setVerbosity(verbosityDetailed);
      
      
      // make XT learner
      SamplerPtr XTexpressionVectorSampler = scalarExpressionVectorSampler();
      SolverPtr XTconditionLearner = randomSplitConditionLearner(XTexpressionVectorSampler);
      //conditionLearner->setVerbosity((SolverVerbosity)verbosity);
      SolverPtr XTlearner = treeLearner(stddevReductionSplittingCriterion(), XTconditionLearner); 
      //learner->setVerbosity((SolverVerbosity)verbosity);
      XTlearner = simpleEnsembleLearner(XTlearner, numTrees);
      XTlearner->setVerbosity(verbosityDetailed);
      
      
      std::vector<SolverPtr> solvers;
      
      solvers.push_back(DTlearner);
      solvers.push_back(RFlearner);
      solvers.push_back(RF2learner);
      solvers.push_back(XTlearner);
      
      
      
      
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
        
        // evaluate
        double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
        context.resultCallback("testingScore", testingScore);
        
        // make curve
        context.enterScope("test");
        for (double x = -1.0; x <= 1.0; x += 0.01)
        {
          context.enterScope(string(x));
          context.resultCallback("x", x);
          context.resultCallback("supervision", targetFunction(x));
          
          ObjectPtr input = new Double(x);
          ObjectPtr prediction = model->compute(context, &input);
          

          // this can be used as an argument to resultcallback
          // ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("some name");
          AggregatorExpressionPtr ensemble = model.dynamicCast<AggregatorExpression>();
          if (ensemble)  // it's an ensemble
          {
            const std::vector<ExpressionPtr>& trees = ensemble->getNodes();
            ScalarVariableStatistics stats;
            for (size_t j = 0; j < numTrees; j++)
            {
              double treepred = Double::get(trees[j]->compute(context, &input));
              //context.resultCallback("tree" + string((int)j), treepred);
              stats.push(treepred);
            }
            
            double stddev = stats.getStandardDeviation();
            double pred = Double::get(prediction);
            //context.resultCallback("predcheck", stats.getMean());
            context.resultCallback("stddevUp", pred + stddev);
            context.resultCallback("stddevDown", pred - stddev);
          }
          context.resultCallback("prediction", prediction);
          context.leaveScope();
        }
        context.leaveScope();
        context.leaveScope();
      }
      
      return new Boolean(true);
    }
    
  protected:
    friend class TestRandomForestUncertaintiesClass;
    
    size_t numSamples;
    size_t numTrees;
    
  private:  
    ProblemPtr makeProblem(ExecutionContext& context)
    {
      ProblemPtr res = new Problem();
      ExpressionDomainPtr domain = new ExpressionDomain();
      VariableExpressionPtr x = domain->addInput(doubleClass, "x");
      VariableExpressionPtr y = domain->createSupervision(doubleClass, "y");
      res->setDomain(domain);
      res->addObjective(normalizedRMSERegressionObjective(makeTable(context, numSamples, x, y), y));
      res->addValidationObjective(normalizedRMSERegressionObjective(makeTable(context, 101, x, y), y));
      return res;
    }
    
    TablePtr makeTable(ExecutionContext& context, size_t count, VariableExpressionPtr x, VariableExpressionPtr y)
    {
      TablePtr res = new Table(count);
      res->addColumn(x, doubleClass);
      res->addColumn(y, doubleClass);
      RandomGeneratorPtr random = context.getRandomGenerator();
      for (size_t i = 0; i < count; ++i)
      {
        double x = random->sampleDouble(-1.0,1.0);
        res->setElement(i, 0, new Double(x));
        double y = targetFunction(x);
        res->setElement(i, 1, new Double(y));
      }
      return res;
    }
    
    double targetFunction(double x)
    {
      return (- x * x + 1);
    }
  };
  
}; /* namespace lbcpp */

#endif // MOO_TEST_RANDOM_FOREST_UNCERTAINTIES_H_
