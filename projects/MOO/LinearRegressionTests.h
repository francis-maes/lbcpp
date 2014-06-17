/*-------------------------------------------.-----------------------------------------.
 | Filename: LinearRegressionTests.h         | Testing Linear Regression Code          |
 | Author  : Denny Verbeeck                  |                                         |
 | Started : 11/06/2014 16:00                |                                         |
 `-------------------------------------------/                                         |
                                  |                                                    |
                                  `---------------------------------------------------*/

#ifndef MOO_LINEAR_REGRESSION_TESTS_H_
# define MOO_LINEAR_REGRESSION_TESTS_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>

// TODO: make attribute limits parameters

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class LinearRegressionTests : public WorkUnit
{
public:
  LinearRegressionTests() : randomSeed(0), numSamples(1000) {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);

    std::vector<IncrementalLearnerPtr> learners;
    learners.push_back(linearLeastSquaresRegressionIncrementalLearner());
    learners.push_back(perceptronIncrementalLearner(50, 0.1, 0.005));
    learners.push_back(simpleLinearRegressionIncrementalLearner());

    std::vector<string> learnerNames;
    learnerNames.push_back("Linear Least Squares");
    learnerNames.push_back("Perceptron");
    learnerNames.push_back("Simple Linear Regression");

    const size_t numAttr = 10;
    TablePtr table = new Table(numSamples);
    TablePtr predictions = new Table(numSamples);
    for (size_t i = 0; i < numAttr; ++i)
      table->addColumn("x" + string((int) i+1), doubleClass);
    table->addColumn("y", doubleClass);
    predictions->addColumn("y", doubleClass);

    std::vector<double> x(numAttr, 0.0);
    for (size_t i = 0; i < numSamples; ++i)
    {
      x[0] = context.getRandomGenerator()->sampleBool() ? -1.0 : 1.0;
      table->setElement(i, 0, new Double(x[0]));
      for (size_t j = 1; j < numAttr; ++j)
      {
        x[j] = (double)context.getRandomGenerator()->sampleInt(-1, 2);
        table->setElement(i, j, new Double(x[j]));
      }
      //x[3] = x[2] + context.getRandomGenerator()->sampleDoubleFromGaussian();
      table->setElement(i, numAttr, new Double(3 + 3 * x[1] + 2 * x[2] + x[3])); // + context.getRandomGenerator()->sampleDoubleFromGaussian()));
      predictions->setElement(i, 0, table->getElement(i, numAttr));
    }

    for (size_t i = 0; i < learners.size(); ++i)
    {
      IncrementalLearnerPtr slr = learners[i];
      ExpressionPtr model = slr->createExpression(context, doubleClass);
      predictions->addColumn(learnerNames[i], doubleClass);
      
      context.enterScope(learnerNames[i]);
      for (size_t j = 0; j < numSamples; ++j)
      {
        context.progressCallback(new ProgressionState(j+1, numSamples, "Samples"));
        slr->addTrainingSample(context, table->getRow(j), model);
      }
      
      
      context.resultCallback("Model", model);
      double rmse = 0.0;
      double error = 0.0;
      ScalarVariableMeanAndVariance residualVariance;
      for (size_t j = 0; j < numSamples; ++j)
      {
        double prediction = Double::get(model->compute(context, table->getRow(j)));
        double value = Double::get(table->getElement(j, numAttr));
        error = prediction - value;
        residualVariance.push(error);
        rmse += error * error;
        predictions->setElement(j, i+1, new Double(prediction));
      }
      rmse /= numSamples;
      rmse = sqrt(rmse);
      context.resultCallback("RMSE", rmse);
      context.resultCallback("Residual Standard Deviation", residualVariance.getStandardDeviation());
      context.leaveScope(rmse);
    }

    context.resultCallback("Data", table);
    context.resultCallback("Predictions", predictions);

    return new Boolean(true);
  }
  
protected:
  friend class LinearRegressionTestsClass;
  
  size_t randomSeed;
  size_t numSamples;
};

} /* namespace lbcpp */

#endif // MOO_LINEAR_REGRESSION_TESTS_H_
