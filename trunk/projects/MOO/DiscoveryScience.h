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
# include "SharkProblems.h"
# include "../../lib/ml/Learner/IncrementalLearnerBasedLearner.h"

// TODO: make attribute limits parameters

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp
  
class DiscoveryScience : public WorkUnit
{
public:
  DiscoveryScience() : randomSeed(456), numSamples(1000), chunkSize(100), verbosity(2) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    

    // set up test problems
    std::vector<ProblemPtr> problems;
    problems.push_back(new FriedmannProblem());
    
    SamplerPtr sampler = uniformSampler();

    std::vector<SolverPtr> learners;
    learners.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(mauveIncrementalSplittingCriterion(0.01, 0.05, 0.95), simpleLinearRegressionIncrementalLearner(), chunkSize)));
    learners.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(0.01, 0.05), perceptronIncrementalLearner(20, 0.1, 0.005), chunkSize)));

    size_t numFolds = 10;
    size_t samplesPerFold = 10000;

    for (size_t functionNumber = 0; functionNumber < problems.size(); ++functionNumber)
    {
      context.enterScope("Function " + string((int) functionNumber));
      // create the learning problem
      ProblemPtr baseProblem = problems[functionNumber];
      sampler->initialize(context, baseProblem->getDomain());
      //ProblemPtr problem = baseProblem->toSupervisedLearningProblem(context, numSamples, 100, sampler);
      std::vector<ProblemPtr> folds = baseProblem->generateFolds(context, numFolds, samplesPerFold, sampler);
      
      for (size_t learnerNb = 0; learnerNb < learners.size(); ++learnerNb)
      {
        double avgTesting = 0.0;
        context.enterScope("Crossvalidation");
        for (size_t foldNb = 0; foldNb < folds.size(); ++foldNb)
        {
          context.progressCallback(new ProgressionState(foldNb, folds.size(), "Folds"));
          ObjectivePtr problemObj = folds[foldNb]->getObjective(0);
          const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
          SolverPtr learner = learners[learnerNb];

          learner->setVerbosity((SolverVerbosity)verbosity);
          learner.staticCast<IncrementalLearnerBasedLearner>()->baseProblem = baseProblem;
        
          ExpressionPtr model;
          FitnessPtr fitness;
          
          context.enterScope("Fold " + string((int) foldNb));
          learner->solve(context, folds[foldNb], storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
          if (verbosity > verbosityDetailed)
          {
            context.resultCallback("model", model);
            context.resultCallback("data", problemData);
          }
          context.resultCallback("fitness", fitness);      
          double testingScore = folds[foldNb]->getValidationObjective(0)->evaluate(context, model);
          context.resultCallback("testingScore", testingScore);
          context.resultCallback("tree size", model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves());
          //makeCurve(context, baseProblem, model);
          //makeMatlabSurface(context, baseProblem, model);
          context.leaveScope(testingScore);
          avgTesting += testingScore;
        }
        context.progressCallback(new ProgressionState(folds.size(), folds.size(), "Folds"));
        avgTesting /= (double)folds.size();
        context.leaveScope(avgTesting);
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
};

}; /* namespace lbcpp */

#endif // !MOO_DISCOVERY_SCIENCE_H_
