/*-----------------------------------------.---------------------------------.
| Filename: TrainMaxentRegularizerWith..cpp| An example that illustrates     |
| Author  : Francis Maes                   |   tuning of an hyper-parameter  |
| Started : 09/06/2009 12:40               |     with cross-validation       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

/*
** Computes 5-fold Cross Validation accuracy
*/
double computeCVAccuracy(ObjectContainerPtr data, StringDictionaryPtr labels, double regularizer, size_t numFolds = 5)
{
  double accuracy = 0.0;
  for (size_t i = 0; i < numFolds; ++i)
  {
    ClassifierPtr classifier = logisticRegressionBinaryClassifier(batchLearner(lbfgsOptimizer()), labels, regularizer);
    classifier->trainBatch(data->fold(i, numFolds));
    accuracy += classifier->evaluateAccuracy(data->invFold(i, numFolds));
  }
  return accuracy / numFolds;
}

int main(int argc, char* argv[])
{
  /*
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  /*
  ** Load classification data
  */
  ObjectContainerPtr data = classificationExamplesParser("../Data/binaryclassif/a1a.train", features, labels)->load(500);
  if (data->empty())
    return 1;
  data = data->randomize();

  /*
  ** Try several values of the regularizer
  */
  double bestAccuracy = 0.0;
  double bestRegularizer = 0.0;
  for (int power = -20; power <= 6; ++power)
  {
    double regularizer = pow(2.0, power);
    double accuracy = computeCVAccuracy(data, labels, regularizer);
    std::cout << "Regularizer: " << regularizer << " CV-Accuracy: " << accuracy * 100 << "%" << std::endl;
    if (accuracy > bestAccuracy)
      bestAccuracy = accuracy, bestRegularizer = regularizer;
  }
  
  /*
  ** Display best regularizer
  */
  std::cout << std::endl
            << "Best Regularizer: " << bestRegularizer
            << " (CV-accuracy: " << bestAccuracy * 100 << "%)"
            << std::endl;
  return 0;
}
