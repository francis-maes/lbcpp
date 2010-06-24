/*-----------------------------------------.---------------------------------.
| Filename: TrainLinearRegressorStocha..cpp| An example to illustrate        |
| Author  : Francis Maes                   |   linear least-squares          |
| Started : 09/06/2009 12:40               |     regression                  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

int main(int argc, char* argv[])
{
  File dataDirectory = File::getCurrentWorkingDirectory().getChildFile("../Data/Regression");

  /*
  ** Create Feature dictionary and Examples parser
  */
  FeatureDictionaryPtr features = new FeatureDictionary("regression-features");
  ObjectStreamPtr parser = regressionExamplesParser(dataDirectory.getChildFile("pyrim.data"), features);
  if (!parser)
    return 1;
  
  /*
  ** Load examples into memory
  */
  ObjectContainerPtr examples = parser->load();
  std::cout << examples->size() << " Examples, " << features->getNumFeatures() << " features." << std::endl;

  /*
  ** Create linear least-squares regressor
  **  with a constant learning rate of 0.01 and a L2-regularizer with weight 0.1
  */
  GradientBasedRegressorPtr regressor = leastSquaresLinearRegressor(stochasticDescentLearner(constantIterationFunction(0.01)), 0.1);
  
  /*
  ** Perform 100 training iterations and evaluate the empirical risk,
  ** the regularized empirical risk and the mean absolute error at each iteration.
  ** Note that examples are randomized before each training iteration.
  */
  for (int i = 0; i < 100; ++i)
  {
    regressor->trainStochastic(examples->randomize());
    std::cout << "Iteration: " << i + 1 << std::endl
              << "  Empirical Risk: " << regressor->computeEmpiricalRisk(examples) << std::endl
              << "  Regularized Empirical Risk: " << regressor->computeRegularizedEmpiricalRisk(examples) << std::endl
              << "  Mean Absolute Error: " << regressor->evaluateMeanAbsoluteError(examples) << std::endl << std::endl;
  }
  return 0;  
}
