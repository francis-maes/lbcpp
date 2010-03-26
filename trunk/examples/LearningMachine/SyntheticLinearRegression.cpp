/*-----------------------------------------.---------------------------------.
| Filename: SyntheticLinearRegression.cpp  | An example that illustrates     |
| Author  : Francis Maes                   |   synthetic learning examples   |
| Started : 08/06/2009 14:52               |    creation and manipulation    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

int main(int argc, char* argv[])
{
  /*
  ** Create a synthetic regression dataset
  **  inputs: sparse vectors with 10 active features out of 1000. Feature values are sampled from a normal Gaussian
  **  outputs: the sum of the input features
  **  examples count: 1000
  */
  VectorObjectContainerPtr examples = new VectorObjectContainer("RegressionExample");
  FeatureDictionaryPtr dictionary = new FeatureDictionary("features");
  for (int i = 0; i < 1000; ++i)
  {
    SparseVectorPtr features = new SparseVector(dictionary);
    double sum = 0.0;
    for (int j = 0; j < 10; ++j)
    {
      double value = RandomGenerator::getInstance().sampleDoubleFromGaussian();
      features->set("feat" + toString(RandomGenerator::getInstance().sampleInt(1000)), value);
      sum += value;
    }
    examples->append(new RegressionExample(features, sum));
  }

  /*
  ** Cross Validation
  */
  const size_t numFolds = 5;
  for (size_t i = 0; i < numFolds; ++i)
  {
    // Create a linear regressor
    RegressorPtr regressor = leastSquaresLinearRegressor(batchLearner(lbfgsOptimizer()));
    regressor->trainBatch(examples->fold(i, numFolds));
    double trainError = regressor->evaluateMeanAbsoluteError(examples->fold(i, numFolds));
    double testError = regressor->evaluateMeanAbsoluteError(examples->invFold(i, numFolds));
    std::cout << "Fold " << (i+1) << "/" << numFolds << ": train = " << trainError << ", test = " << testError << std::endl;
  }
  return 0;
}
