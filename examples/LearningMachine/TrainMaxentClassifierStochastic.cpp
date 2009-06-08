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
  /*
  ** Create Feature dictionary and Labels dictionary
  */
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();

  /*
  ** Load training classification data
  */
  ObjectStreamPtr parser = classificationExamplesParser("../data/classification/small.train", features, labels);
  if (!parser->isValid())
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
    classifier->trainStochastic(trainingData);
    std::cout << "Iteration " << (i+1)
              << " Training Accuracy: " << classifier->evaluateAccuracy(trainingData) * 100 << "%."
              << std::endl;
  }
  
  /*
  ** Parse test data and evaluate testing accuracy in one pass
  */
  parser = classificationExamplesParser("../data/classification/small.test", features, labels);
  if (!parser->isValid())
    return 1;
  std::cout << "Testing Accuracy: " << classifier->evaluateAccuracy(parser) * 100 << "%." << std::endl;

  /*
  ** Save the classifier
  */
  classifier->saveToFile("classifier.model"); 
  return 0;
}
