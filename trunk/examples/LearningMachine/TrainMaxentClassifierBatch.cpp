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
  File dataDirectory = File::getCurrentWorkingDirectory().getChildFile("../Data/Classification");
  
  /*
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  FeatureDictionaryPtr labels = new FeatureDictionary("labels");

  /*
  ** Load training classification data
  */
  ObjectStreamPtr parser = classificationExamplesParser(dataDirectory.getChildFile("small.train"), features, labels);
  if (parser->isExhausted())
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
  parser = classificationExamplesParser(dataDirectory.getChildFile("small.test"), features, labels);
  if (parser->isExhausted())
    return 1;
  std::cout << "Testing Accuracy: " << classifier->evaluateAccuracy(parser) * 100 << "%." << std::endl;

  /*
  ** Save the classifier
  */
  File directory = File::getCurrentWorkingDirectory();
  classifier->saveToFile(directory.getChildFile("classifier.model"));
  classifier->getParameters()->saveToFile(directory.getChildFile("parameters.vector"));
  features->saveToFile(directory.getChildFile("features.dic"));
  return 0;
}
