/*-----------------------------------------.---------------------------------.
| Filename: BasicOperations.cpp            | An example that illustrates     |
| Author  : Francis Maes                   |   basic operations on           |
| Started : 08/06/2009 14:52               |    learning machines.           |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

//GradientBasedClassifierPtr classifier = maximumEntropyClassifier(batchLearner(lbfgsOptimizer()), labels);

int main(int argc, char* argv[])
{
  // First Step: loading the data
  FeatureDictionaryPtr features = new FeatureDictionary("features");
  StringDictionaryPtr labels = new StringDictionary();
  ObjectStreamPtr parser = classificationExamplesParser("../data/classification/small.train", features, labels);
  if (!parser)
    return 1;
  ObjectContainerPtr trainingData = parser->load();
  trainingData = trainingData->randomize();
  std::cout << trainingData->size() << " training examples of class " << trainingData->getContentClassName() << std::endl;
  
  // Second Step: learning a classifier
  ClassifierPtr classifier = maximumEntropyClassifier(batchLearner(lbfgsOptimizer()), labels);
  classifier->trainBatch(trainingData, &ProgressCallback::getConsoleProgressCallback());
  
  std::cout << "Training Accuracy: " << classifier->evaluateAccuracy(trainingData->toStream()) * 100 << "%." << std::endl;
  
  // Third Step: saving the classifier
  classifier->saveToFile("classifier.model"); 
  return 0;
}
