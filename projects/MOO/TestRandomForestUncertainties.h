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
# include <ml/SelectionCriterion.h>

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class TestRandomForestUncertainties : public WorkUnit
{
public:
  TestRandomForestUncertainties() : numSamples(25), numTrees(100), numObjectives(1), randomSeed(0) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    
    moFunctionIdx.resize(numObjectives);
    for (size_t i = 0; i < numObjectives; ++i)
       moFunctionIdx[i] = i;
    
    if (numObjectives > 8)
    {
      context.errorCallback("Too many objectives, maximum is 8");
      return new Boolean(false);
    }
    

    // create the domain
    ExpressionDomainPtr domain = new ExpressionDomain();
    VariableExpressionPtr x = domain->addInput(doubleClass, "x");
    VariableExpressionPtr y;
    
    if (numObjectives == 1)
     y = domain->createSupervision(doubleClass, "y");
    else
    {
      DefaultEnumerationPtr objectivesEnumeration = new DefaultEnumeration("objectives");
      for (size_t i = 0; i < numObjectives; ++i)
        objectivesEnumeration->addElement(context, "objective" + string((int)i));
      ClassPtr dvClass = denseDoubleVectorClass(objectivesEnumeration, doubleClass);
      y = domain->createSupervision(dvClass, "y");
    }
    
    // create the learning problem
    ProblemPtr problem = makeProblem(context, domain);
    
    // create the splitting criterion
    SplittingCriterionPtr splittingCriterion;
    if (numObjectives == 1)
      splittingCriterion = stddevReductionSplittingCriterion();
    else
      splittingCriterion = vectorStddevReductionSplittingCriterion();
    
    // create the sampler
    SamplerPtr sampler = scalarExpressionVectorSampler();
    
    // create condition learner for decision trees and random forests
    SolverPtr exhaustive = exhaustiveConditionLearner(sampler);
         
    // create DT learner
    SolverPtr dtLearner = treeLearner(splittingCriterion, exhaustive);
    dtLearner->setVerbosity(verbosityDetailed);

    // create RF learner
    SolverPtr rfLearner = treeLearner(splittingCriterion, exhaustive); 
    rfLearner = baggingLearner(rfLearner, numTrees);
    rfLearner->setVerbosity(verbosityDetailed);
    
    
    // create XT learner
    // these trees should choose random splits 
    SolverPtr xtLearner = treeLearner(splittingCriterion, randomSplitConditionLearner(sampler)); 
    xtLearner = simpleEnsembleLearner(xtLearner, numTrees);
    xtLearner->setVerbosity(verbosityDetailed);
    
    
    // put learners in a vector
    std::vector<SolverPtr> solvers;
    
    //solvers.push_back(dtLearner);
    solvers.push_back(rfLearner);
    solvers.push_back(xtLearner);
    
    
    ObjectivePtr problemObj = problem->getObjective(0);
    const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    
    TablePtr testTable = makeTestTable(domain->getInput(0));

    
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
      if (numObjectives == 1)
      {
        FitnessLimitsPtr limits = new FitnessLimits();
        double worst, best;
        problem->getObjective(0)->getObjectiveRange(worst, best);
        limits->addObjective(worst, best);
        for (size_t r = 0; r < problemData->getNumRows(); r++)
        {
          ObjectPtr supervision = problemData->getElement(r, 1);
          FitnessPtr fitnessR = new Fitness(getIthValue(supervision, 0), limits);
          bestFitness = (!bestFitness || fitnessR->dominates(bestFitness) ? fitnessR : bestFitness);
        }
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
          
          AggregatorExpressionPtr ensemble = model.dynamicCast<AggregatorExpression>();
          if (ensemble)  // it's an ensemble
          {
            ScalarVariableStatisticsPtr prediction;
            if (numObjectives == 1)
              prediction = predictions->getElement(i).staticCast<ScalarVariableStatistics>();
            else
            {
              OVectorPtr multiPrediction = predictions->getElement(i).staticCast<OVector>();
              prediction = multiPrediction->getAndCast<ScalarVariableStatistics>(j);
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
    }
    
    return new Boolean(true);
  }
  
protected:
  friend class TestRandomForestUncertaintiesClass;
  
  size_t numSamples;
  size_t numTrees;
  size_t numObjectives;
  
  int randomSeed;
  
private:
  
  std::vector<size_t> moFunctionIdx;
  
  double getIthValue(ObjectPtr object, size_t i) const
    {return (numObjectives == 1 ? Double::get(object) : object.staticCast<DenseDoubleVector>()->getValue(i));}
    
  ProblemPtr makeProblem(ExecutionContext& context, ExpressionDomainPtr domain)
  {
    ProblemPtr res = new Problem();
    
    VariableExpressionPtr x = domain->getInput(0);
    VariableExpressionPtr y = domain->getSupervision();
    
    res->setDomain(domain);

    if (numObjectives == 1) {
      res->addObjective(mseRegressionObjective(makeTable(context, numSamples, x, y), y));
      res->addValidationObjective(mseRegressionObjective(makeTable(context, 101, x, y), y));
    }
    else
    {
      res->addObjective(mseMultiRegressionObjective(makeTable(context, numSamples, x, y), y));
      res->addValidationObjective(mseMultiRegressionObjective(makeTable(context, 101, x, y), y));
    }
    return res;
  }
  
  TablePtr makeTable(ExecutionContext& context, size_t count, VariableExpressionPtr x, VariableExpressionPtr y)
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
      
      if (numObjectives == 1)
      {
        double y = targetFunction(x, moFunctionIdx[0]);
        res->setElement(i, 1, new Double(y));
      }
      else
      {
        DenseDoubleVectorPtr targetVector(new DenseDoubleVector(y->getType()));
        for (size_t j = 0; j < numObjectives; ++j)
          targetVector->setValue(j, targetFunction(x, moFunctionIdx[j]));
        res->setElement(i, 1, targetVector);
      }
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
  
  double targetFunction(double x, size_t functionIndex)
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
    }
    jassert(false);
    return 0.0;
  }
};

}; /* namespace lbcpp */

#endif // MOO_TEST_RANDOM_FOREST_UNCERTAINTIES_H_
