/*---------------------------------------------.-----------------------------------------.
 | Filename: Sandbox.h                         | Sandbox environment for testing purposes|
 | Author  : Denny Verbeeck                    |                                         |
 | Started : 18/06/2014 17:00                  |                                         |
 `---------------------------------------------/                                         |
                                  |                                                      |
                                  `-----------------------------------------------------*/

#ifndef MOO_DBG_SANDBOX_H_
# define MOO_DBG_SANDBOX_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/IncrementalLearner.h>
# include <ml/BinarySearchTree.h>
# include <ml/Expression.h>
# include <ml/Sampler.h>
# include <ml/RandomVariable.h>
# include "SolverInfo.h"
# include "SharkProblems.h"

namespace lbcpp
{

class Sandbox : public WorkUnit
{
public:
  Sandbox() : datasetPath("") {}

  virtual ObjectPtr run(ExecutionContext& context)
  {

    MultiVariateRegressionStatisticsPtr stats = new MultiVariateRegressionStatistics();
    std::vector<DenseDoubleVectorPtr> X;
    std::vector<double> Y;
    size_t m = 10; //num attributes
    size_t n = 200; // num examples
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr x = new DenseDoubleVector(4, 0.0);
      for (size_t j = 0; j < m; ++j)
        x->setValue(j, context.getRandomGenerator()->sampleDouble());
      double y = x->getValue(0) + 2*x->getValue(1) - x->getValue(2) - 2*x->getValue(3) + context.getRandomGenerator()->sampleDoubleFromGaussian();
      stats->push(x, y);
      X.push_back(x);
      Y.push_back(y);
      DenseDoubleVectorPtr weights = stats->getLLSQEstimate();
      if (i > 2)
      {
        context.enterScope("Iteration " + string((int) i));
        context.resultCallback("Iteration", i);
        context.resultCallback("Calculated RSD", rsd(X, Y, weights));
        context.resultCallback("Estimated RSD", stats->getResidualStandardDeviation());
        context.leaveScope();
      }
    }

    /*
    std::vector< std::pair<juce::String, IncrementalLearnerPtr> > learners;
    learners.push_back(std::make_pair("LLSQ", linearLeastSquaresRegressionIncrementalLearner()));
    learners.push_back(std::make_pair("SLR", simpleLinearRegressionIncrementalLearner()));
    //learners.push_back(std::make_pair("Perceptron", perceptronIncrementalLearner(20, 0.05, 0.01)));

    const size_t numSamples = 10000;

    std::vector<std::pair<string, ProblemPtr> > problems = createProblems(context, numSamples, numSamples / 10, false, datasetPath);

    SolverPtr solver = incrementalLearnerBasedLearner(linearLeastSquaresRegressionIncrementalLearner());
    for (size_t i = 0; i < problems.size(); ++i)
    {
      ExpressionPtr model;
      FitnessPtr fitness;
      context.enterScope(problems[i].first);
      solver->solve(context, problems[i].second, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
      context.resultCallback("Model", model);
      context.leaveScope(fitness);
    }


    /*const size_t numPoints = 10000;
    DenseDoubleVectorPtr input = new DenseDoubleVector(2, 0.0);
    DenseDoubleVectorPtr output = new DenseDoubleVector(1, 0.0);
    FitnessPtr fitness;
    context.enterScope("Learning weights");
    for (size_t i = 0; i < numPoints; ++i)
    {
      input->setValue(0, context.getRandomGenerator()->sampleDouble(0.0, 2.0));
      input->setValue(1, context.getRandomGenerator()->sampleDouble(0.0, 2.0));
      fitness = problem->evaluate(context, input);
      output->setValue(0, fitness->getValue(0));
      context.enterScope("Iteration " + string((int) i));
      context.resultCallback("iteration", i);
      for (size_t l = 0; l < learners.size(); ++l)
      {
        learners[l].second->addTrainingSample(context, models[l], input, output);
        DenseDoubleVectorPtr& weights = models[l]->getWeights();
        for (size_t j = 0; j < weights->getNumValues(); ++j)
          context.resultCallback(learners[l].first + string((int) j), weights->getValue(j));
      }
      context.leaveScope();
      context.progressCallback(new ProgressionState(i + 1, numPoints, "Iterations"));
    }
    context.leaveScope();*/
    return new Boolean(true);
  }

  double rsd(std::vector<DenseDoubleVectorPtr> X, std::vector<double> Y, DenseDoubleVectorPtr weights)
  {
    double sumResiduals = 0.0;
    double sumSqResiduals = 0.0;
    for (size_t i = 0; i < X.size(); ++i)
    {
      double yhat = weights->getValue(0);
      for (size_t j = 0; j < X[0]->getNumValues(); ++j)
        yhat += X[i]->getValue(j) * weights->getValue(j+1);
      sumResiduals += yhat - Y[i];
      sumSqResiduals += (yhat - Y[i]) * (yhat - Y[i]);
    }
    return sqrt(sumSqResiduals / X.size() - (sumResiduals / X.size()) * (sumResiduals / X.size()));
  }

  std::vector<std::pair<string, ProblemPtr> > createProblems(ExecutionContext& context, size_t numSamples, size_t numValidationSamples, bool testRun, string arffPath)
  {
    // set up test problems
    std::vector<std::pair<string, ProblemPtr> > problems;
    problems.push_back(std::make_pair("Friedmann", (new FriedmannProblem())->toSupervisedLearningProblem(context, numSamples, numValidationSamples, uniformSampler())));

    if (!testRun)
    {
      problems.push_back(std::make_pair("Paraboloid", (new ParaboloidProblem())->toSupervisedLearningProblem(context, numSamples, numValidationSamples, uniformSampler())));
      problems.push_back(std::make_pair("Lexp", (new LEXPProblem())->toSupervisedLearningProblem(context, numSamples, numValidationSamples, uniformSampler())));
      problems.push_back(std::make_pair("Losc", (new LOSCProblem())->toSupervisedLearningProblem(context, numSamples, numValidationSamples, uniformSampler())));
    }

    /*
    SamplerPtr sampler = uniformSampler();
    std::vector< std::vector<ProblemPtr> > xValProblems;
    for (size_t i = 0; i < problems.size(); ++i)
    {
      sampler->initialize(context, problems[i]->getDomain());
      xValProblems.push_back(problems[i]->generateFolds(context, 10, numSamples / 10, sampler));
      context.informationCallback("Finished creating " + problemnames[i]);
    }
    */
    if (!testRun)
    {
      std::vector<string> datasets;
      datasets.push_back("fried_delve.arff");
      datasets.push_back("cart_delve.arff");
      datasets.push_back("pol.arff");
      datasets.push_back("winequality-white.arff");
      datasets.push_back("cal_housing.arff");
      //datasets.push_back("CASP.arff");
    
      std::vector<string> problemnames;
      problemnames.push_back("Friedmann");
      problemnames.push_back("CART");
      problemnames.push_back("PoleTelecom");
      problemnames.push_back("Wine quality");
      problemnames.push_back("California housing");
      //problemnames.push_back("Physicochemical Properties of Protein Tertiary Structure");
    
      ArffLoader loader;
      for (size_t i = 0; i < datasets.size(); ++i)
      {
        TablePtr table = loader.loadFromFile(context, juce::File(arffPath + "/" + datasets[i])).staticCast<Table>();
        context.informationCallback("Finished loading " + datasets[i]);
        problems.push_back(std::make_pair(problemnames[i], Problem::fromTable(context, table, 0.0)));
      }
    }
    return problems;
  }

protected:
  friend class SandboxClass;

  string datasetPath;

};

}


#endif //!MOO_DBG_SANDBOX_H_
