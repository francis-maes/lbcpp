/*-----------------------------------------.---------------------------------.
| Filename: LoadMaxentClassifier.cpp       | An example to show how to load  |
| Author  : Francis Maes                   |  a classifier from a file       |
| Started : 08/06/2009 21:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

int main(int argc, char* argv[])
{
  /*
  ** Load a Classifier from file "classifier.model"
  */
  GradientBasedClassifierPtr classifier = Object::loadFromFileCast<GradientBasedClassifier>("classifier.model");
  assert(classifier);

  /*
  ** Evaluate training accuracy
  */
  double accuracy = classifier->evaluateAccuracy(
    classificationExamplesParser("../data/classification/small.train", classifier->getInputDictionary(), classifier->getLabels()));
  std::cout << "Training Accuracy: " << accuracy * 100 << "%." << std::endl;

  /*
  ** Evaluate testing accuracy
  */
  accuracy = classifier->evaluateAccuracy(
    classificationExamplesParser("../data/classification/small.test", classifier->getInputDictionary(), classifier->getLabels()));
  std::cout << "Testing Accuracy: " << accuracy * 100 << "%." << std::endl;
  return 0;
}
