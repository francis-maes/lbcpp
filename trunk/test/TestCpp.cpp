#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
#include <fstream>
using namespace cralgo;

int main(int argc, char* argv[])
{
//  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/binaryclassif/a1a.test";
    static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/classification/small.train";

  std::ifstream istr(filename);
  if (!istr.is_open())
  {
    std::cerr << "Could not open file " << filename << std::endl;
    return 1;
  }

  FeatureDictionary features("dictionary");
  FeatureDictionary labels("labels");
  std::vector<ClassificationExample> examples;
  if (!parseClassificationExamples(istr, features, labels, examples))
    return 1;

  std::cout << examples.size() << " Examples." << std::endl << toString(features) << std::endl << "Labels: " << toString(labels) << std::endl;

  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(GradientBasedLearner::createGradientDescent(), labels);

//  BinaryClassifierPtr classifier = GradientBasedBinaryClassifier::createLogisticRegression(GradientBasedLearner::createGradientDescent(), labels);
  
  for (int i = 0; i < 10; ++i)
  {
    classifier->trainStochastic(examples);

//  classifier->trainBatch(examples);

    size_t numCorrect = 0;
    for (size_t i = 0; i < examples.size(); ++i)
      if (classifier->predict(examples[i].getInput()) == examples[i].getOutput())
        ++numCorrect;
    std::cout << "Accuracy: " << (100.0 * numCorrect / (double)examples.size()) << "%" << std::endl;
  }  
  return 0;
}