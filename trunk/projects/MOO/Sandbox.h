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

namespace lbcpp
{

class Sandbox : public WorkUnit
{
public:
  Sandbox() {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    IncrementalLearnerPtr learner = hoeffdingTreeIncrementalLearner(hoeffdingBoundMauveIncrementalSplittingCriterion(0.01, 0.05), linearLeastSquaresRegressionIncrementalLearner(), 11);
    ExpressionPtr model = learner->createExpression(context, doubleClass);
    for (size_t i = 0; i < 12; ++i)
    {
      double x = (double) i;
      double y = (x <= 5.0 ? x : 10 - x) + context.getRandomGenerator()->sampleDoubleFromGaussian(0.0, 0.2);
      DenseDoubleVectorPtr input = new DenseDoubleVector(1, x);
      DenseDoubleVectorPtr output = new DenseDoubleVector(1, y);
      learner->addTrainingSample(context, model, input, output);
    }
    context.resultCallback("model", model);
    return new Boolean(true);
  }


};

}


#endif //!MOO_DBG_SANDBOX_H_
