/*---------------------------------------------.-----------------------------------------.
 | Filename: DiscoveryScience.h                | Experiments for Discovery Science 2014  |
 | Author  : Denny Verbeeck                    |                                         |
 | Started : 18/04/2014 11:25                  |                                         |
 `---------------------------------------------/                                         |
                                  |                                                      |
                                  `-----------------------------------------------------*/

#ifndef MOO_DISCOVERY_SCIENCE_H_
# define MOO_DISCOVERY_SCIENCE_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>
# include <ml/Sampler.h>
# include <math.h>
# include "SharkProblems.h"
# include "../../lib/ml/Learner/IncrementalLearnerBasedLearner.h"
# include "../../lib/ml/Loader/ArffLoader.h"

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class DiscoveryScience : public WorkUnit
{
public:
  DiscoveryScience() : randomSeed(456), numSamples(1000), chunkSize(100), verbosity(2), datasetPath("") {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    

    // set up test problems
    std::vector<ProblemPtr> problems;
    std::vector<string> artificialNames;
    problems.push_back(new FriedmannProblem());
    problems.push_back(new LEXPProblem());
    problems.push_back(new LOSCProblem());
    artificialNames.push_back("Friedmann");
    artificialNames.push_back("Lexp");
    artificialNames.push_back("Losc");

    SamplerPtr sampler = uniformSampler();

    
    std::vector<string> algoNames;
    
    
    algoNames.push_back("FIMT");
    algoNames.push_back("iMauve");
    

    std::vector<string> datasets;
    datasets.push_back("winequality-white.arff");
    datasets.push_back("cal_housing.arff");
    datasets.push_back("CASP.arff");
    std::vector<string> datasetnames;
    datasetnames.push_back("Wine quality");
    datasetnames.push_back("California housing");
    datasetnames.push_back("Physicochemical Properties of Protein Tertiary Structure");

    size_t numFolds = 10;
    size_t samplesPerFold = 1000;
    ArffLoader loader;
    
    std::vector<double> deltas;
    deltas.push_back(0.01);
    deltas.push_back(0.05);
    deltas.push_back(0.10);
    deltas.push_back(0.15);
    deltas.push_back(0.20);
    deltas.push_back(0.25);
    
    for (size_t d = 0; d < deltas.size(); ++d)
    {
      context.enterScope("delta = " + string(deltas[d]));
      std::vector<SolverPtr> learners;
      learners.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(deltas[d], 0.05), perceptronIncrementalLearner(20, 0.1, 0.005), chunkSize)));
      learners.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(mauveIncrementalSplittingCriterion(deltas[d], 0.05, 2.0), simpleLinearRegressionIncrementalLearner(), chunkSize)));
      for (size_t learnerNb = 0; learnerNb < learners.size(); ++learnerNb)
          learners[learnerNb]->setVerbosity((SolverVerbosity) verbosity);

      for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
      {
        context.enterScope(artificialNames[functionNumber]);
        // create the learning problem
        ProblemPtr baseProblem = problems[functionNumber];
        sampler->initialize(context, baseProblem->getDomain());
      
        //ProblemPtr problem = baseProblem->toSupervisedLearningProblem(context, numSamples, 0.1*numSamples, sampler);
        for (size_t learnerNb = 0; learnerNb < learners.size(); ++learnerNb)
        {
          doCrossValidation(context, baseProblem->generateFolds(context, 10, numSamples / 10, sampler), learners[learnerNb], algoNames[learnerNb]);
          /*
          context.enterScope(algoNames[learnerNb]);
          ExpressionPtr model = solveProblem(context, problem, learners[learnerNb]);
      
          double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
          size_t nbLeaves = model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves();
          context.resultCallback("testingScore", testingScore);
          context.resultCallback("tree size", nbLeaves);
          
          //context.resultCallback("model", model);
          context.leaveScope(testingScore);*/
        }

        /*
        std::vector<ProblemPtr> folds = baseProblem->generateFolds(context, numFolds, samplesPerFold, sampler);
        for (size_t learnerNb = 0; learnerNb < learners.size(); ++learnerNb)
          doCrossValidation(context, folds, learners[learnerNb], algoNames[learnerNb]);
        */

        context.leaveScope();
      }
    
      for (size_t datasetNb = 0; datasetNb < datasets.size(); ++datasetNb)
      {
        context.enterScope(datasetnames[datasetNb]);
        TablePtr table = loader.loadFromFile(context, juce::File(datasetPath + "/" + datasets[datasetNb])).staticCast<Table>();
        ProblemPtr problem = Problem::fromTable(context, table, 0.1);
        for (size_t learnerNb = 0; learnerNb < learners.size(); ++learnerNb)
        {
          doCrossValidation(context, Problem::generateFoldsFromTable(context, table, 10), learners[learnerNb], algoNames[learnerNb]);
          /*
          context.enterScope(algoNames[learnerNb]);
          ExpressionPtr model = solveProblem(context, problem, learners[learnerNb]);
      
          double testingScore = problem->getValidationObjective(0)->evaluate(context, model);
          size_t nbLeaves = model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves();
          context.resultCallback("testingScore", testingScore);
          context.resultCallback("tree size", nbLeaves);
          //context.resultCallback("model", model);
          context.leaveScope(testingScore);*/
        }
        context.leaveScope();
      }
      context.leaveScope();
    }
    return new Boolean(true);
  }
  
protected:
  friend class DiscoveryScienceClass;
  
  size_t numSamples;
  size_t randomSeed;
  size_t chunkSize;
  double learningRate;
  double learningRateDecay;
  double delta;
  double threshold;
  size_t numDims;
  size_t verbosity;

  string datasetPath;

private:

  ExpressionPtr solveProblem(ExecutionContext& context, const ProblemPtr& problem, const SolverPtr& learner)
  {
    ExpressionPtr model;
    FitnessPtr fitness;
    ObjectivePtr problemObj = problem->getObjective(0);
    const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
    if (verbosity > verbosityDetailed)
    {
      context.resultCallback("model", model);
      context.resultCallback("data", problemData);
    }
    context.resultCallback("fitness", fitness);      
    return model;
  }

  void doCrossValidation(ExecutionContext& context, const std::vector<ProblemPtr>& folds, SolverPtr learner, string algoName)
  {
    ScalarVariableMean meanTesting;
    ScalarVariableMean meanTreeSize;
    context.enterScope(algoName);
    for (size_t foldNb = 0; foldNb < folds.size(); ++foldNb)
    {
      context.progressCallback(new ProgressionState(foldNb, folds.size(), "Folds"));
      context.enterScope("Fold " + string((int) foldNb));
      ExpressionPtr model = solveProblem(context, folds[foldNb], learner);
      
      double testingScore = folds[foldNb]->getValidationObjective(0)->evaluate(context, model);
      size_t nbLeaves = model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves();
      context.resultCallback("testingScore", testingScore);
      context.resultCallback("tree size", nbLeaves);
      context.leaveScope(testingScore);
      meanTesting.push(testingScore);
      meanTreeSize.push(nbLeaves);
    }
    context.progressCallback(new ProgressionState(folds.size(), folds.size(), "Folds"));
    context.resultCallback("Mean testing score", meanTesting.getMean());
    context.resultCallback("Mean tree size", meanTreeSize.getMean());
    context.leaveScope(meanTesting.getMean());
  }

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
};

}; /* namespace lbcpp */

#endif // !MOO_DISCOVERY_SCIENCE_H_
