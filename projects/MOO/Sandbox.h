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

namespace lbcpp
{

class Sandbox : public WorkUnit
{
public:
  Sandbox() {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    std::vector< std::pair<juce::String, IncrementalLearnerPtr> > learners;
    learners.push_back(std::make_pair("LLSQ", linearLeastSquaresRegressionIncrementalLearner()));
    learners.push_back(std::make_pair("SLR", simpleLinearRegressionIncrementalLearner()));
    //learners.push_back(std::make_pair("Perceptron", perceptronIncrementalLearner(20, 0.05, 0.01)));

    std::vector<LinearModelExpressionPtr> models;
    for (size_t i = 0; i < learners.size(); ++i)
      models.push_back(learners[i].second->createExpression(context, doubleClass).staticCast<LinearModelExpression>());

    ProblemPtr problem = new ParaboloidProblem();

    const size_t numPoints = 10000;
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
    context.leaveScope();
    return new Boolean(true);
  }


};

}


#endif //!MOO_DBG_SANDBOX_H_
