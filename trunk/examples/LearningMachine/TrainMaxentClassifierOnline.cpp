/*-----------------------------------------.---------------------------------.
| Filename: TrainMaxentClassifierOnline.cpp| An example that illustrates     |
| Author  : Francis Maes                   |  online (one pass) training     |
| Started : 08/06/2009 20:41               |                                 |
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
  ** Create a maximum-entropy classifier with stochastic gradient descent, with a (normalized) learning rate of 10.0
  */
  ClassifierPtr classifier = maximumEntropyClassifier(stochasticDescentLearner(constantIterationFunction(10.0)), labels, 0.001);

  /*
  ** Create classification data parser and perform online training
  */
  ObjectStreamPtr parser = classificationExamplesParser("../data/classification/small.train", features, labels);
  if (!parser->isValid())
    return 1;
  classifier->trainStochastic(parser);
  
  /*
  ** Evaluate testing accuracy
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
