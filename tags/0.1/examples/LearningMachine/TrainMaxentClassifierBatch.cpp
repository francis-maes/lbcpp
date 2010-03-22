/*-----------------------------------------.---------------------------------.
| Filename: TrainMaxentClassifierBatch.cpp | An example that illustrates     |
| Author  : Francis Maes                   |   batch training of a classifier|
| Started : 08/06/2009 14:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

int main(int argc, char* argv[])
{
  /*
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  /*
  ** Load training classification data
  */
  ObjectStreamPtr parser = classificationExamplesParser("../Data/Classification/small.train", features, labels);
  if (!parser->isValid())
    return 1;
  ObjectContainerPtr trainingData = parser->load();
  std::cout << "Labels: " << labels->toString() << std::endl;
  
  /*
  ** Create a maximum-entropy classifier (training with LBFGS, 20 iterations max, L2 regularizer: 0.001)
  */
  GradientBasedClassifierPtr classifier = maximumEntropyClassifier(batchLearner(lbfgsOptimizer(), 20), labels, 0.001);

  /*
  ** Perform batch training with LBFGS
  */
  static const bool verbose = true;
  if (verbose)
    classifier->trainBatch(trainingData, consoleProgressCallback());
  else
    classifier->trainBatch(trainingData);
  
  /*
  ** Evaluate training accuracy
  */
  std::cout << "Training Accuracy: " << classifier->evaluateAccuracy(trainingData) * 100 << "%." << std::endl;
  
  /*
  ** Parse test data and evaluate testing accuracy in one pass
  */
  parser = classificationExamplesParser("../Data/Classification/small.test", features, labels);
  if (!parser->isValid())
    return 1;
  std::cout << "Testing Accuracy: " << classifier->evaluateAccuracy(parser) * 100 << "%." << std::endl;

  /*
  ** Save the classifier
  */
  classifier->saveToFile("classifier.model"); 
  classifier->getParameters()->saveToFile("parameters.vector");
  features->saveToFile("features.dic");
  return 0;
}
