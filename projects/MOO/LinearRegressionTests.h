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

    IncrementalLearnerPtr slr = linearLeastSquaresRegressionIncrementalLearner();
    ExpressionPtr model = slr->createExpression(context, doubleClass);

    const size_t numAttr = 10;
    DenseDoubleVectorPtr x = new DenseDoubleVector(numAttr, 0.0);
    DenseDoubleVectorPtr y = new DenseDoubleVector(1, 0.0);
    TablePtr table = new Table(numSamples);
    for (size_t i = 0; i < numAttr; ++i)
      table->addColumn("x" + string((int) i+1), doubleClass);
    table->addColumn("y", doubleClass);
    table->addColumn("prediction", doubleClass);
    for (size_t i = 0; i < numSamples; ++i)
    {
      x->setValue(0, context.getRandomGenerator()->sampleInt(0, 2));
      table->setElement(i, 0, new Double(x->getValue(0)));
      for (size_t j = 1; j < numAttr; ++j)
      {
        x->setValue(j, context.getRandomGenerator()->sampleInt(-1, 2));
        table->setElement(i, j, new Double(x->getValue(j)));
      }
      x->setValue(2, x->getValue(3));
      y->setValue(0, 3 + 3 * x->getValue(1) + 2 * x->getValue(2) + x->getValue(3)); // + context.getRandomGenerator()->sampleDoubleFromGaussian());
      table->setElement(i, numAttr, new Double(y->getValue(0)));
      slr->addTrainingSample(context, model, x, y);
    }
    for (size_t i = 0; i < numSamples; ++i)
      table->setElement(i, numAttr + 1, model->compute(context, table->getRow(i)));
    context.resultCallback("Weights", model.staticCast<LinearModelExpression>()->getWeights());
    context.resultCallback("Data", table);
    
    return new Boolean(true);
  }
  
protected:
  friend class LinearRegressionTestsClass;
  
  size_t randomSeed;
  size_t numSamples;
};

} /* namespace lbcpp */

#endif // MOO_LINEAR_REGRESSION_TESTS_H_
