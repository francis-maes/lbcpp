/*-----------------------------------------.---------------------------------.
| Filename: TrainMaxentClassifierStoch..cpp| An example that illustrates     |
| Author  : Francis Maes                   |   stochastic training of        |
| Started : 08/06/2009 20:49               |     a classifier                |
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
  
  /*
  ** Create a maximum-entropy classifier (training with stochastic descent, L2 regularizer: 1)
  */
  GradientBasedLearnerPtr learner = stochasticDescentLearner(constantIterationFunction(10.0));
  ClassifierPtr classifier = maximumEntropyClassifier(learner, labels, 1);

  /*
  ** Perform training for 5 iterations
  */
  for (size_t i = 0; i < 5; ++i)
  {
    classifier->trainStochastic(trainingData->randomize());
    std::cout << "Iteration " << (i+1)
              << " Training Accuracy: " << classifier->evaluateAccuracy(trainingData) * 100 << "%."
              << std::endl;
  }
  
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
  classifier->saveToFile(File::getCurrentWorkingDirectory().getChildFile("classifier.model"));
  return 0;
}
