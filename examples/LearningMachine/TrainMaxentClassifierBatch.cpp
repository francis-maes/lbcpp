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
  ObjectStreamPtr parser = ObjectStream::createClassificationExamplesParser("../data/classification/small.train", features, labels);
  if (!parser)
    return 1;
  ObjectContainerPtr trainingData = parser->load(50);
  trainingData = trainingData->randomize();
  std::cout << trainingData->size() << " training examples of class " << trainingData->getContentClassName() << std::endl;
  
  // Second Step: learning a classifier
  GradientBasedLearnerPtr learner = GradientBasedLearner::createBatch(VectorOptimizer::createLBFGS());
  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(learner, labels);
  classifier->trainBatch(trainingData);
  
  std::cout << "Training Accuracy: " << classifier->evaluateAccuracy(trainingData->toStream()) * 100 << "%." << std::endl;
  
  // Third Step: saving the classifier
  classifier->saveToFile("classifier.model"); 
  
/*
  // First Step: loading the data
  InstanceSet data = InstanceSet::loadClassificationData("../data/classification/small.train");
  if (!data.exists())
  {
    std::cerr << "Could not load classification data." << std::endl;
    return 1;
  }
  data = data.randomize();

  // Second Step: learning a classifier
  LearningMachine machine = EnergyBasedMachine::createMaxentClassifier();
  machine.train(data);

  // Third Step: saving and loading the classifier
  machine.save("example.model");

  // - the rest could be another program ...
  machine = LearningMachine::load("example.model");
  if (!machine.exists())
  {
    std::cout << "Could not load the model." << std::endl;
    return 1;
  }

  // Fourth Step: Evaluating the classifier on new examples
  InstanceSet testdata = InstanceSet::loadClassificationData("../data/classification/small.test");
  Frame results = machine.evaluate(testdata);
  std::cout << "Generalization Accuracy: " << results.getDouble("accuracy") << std::endl;
  std::cout << "Train Accuracy: " << machine.evaluate(data).getDouble("accuracy") << std::endl;
  
  nieme::uninitialize();*/
  return 0;
}
