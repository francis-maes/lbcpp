#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
#include <fstream>
using namespace cralgo;

int main(int argc, char* argv[])
{
  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/binaryclassif/a1a.train";
//  static const char* filename = "/Users/francis/Projets/Francis/data/sequences/NER-small.test";
 // static const char* filename = "/Users/francis/Projets/Francis/data/sequences/NER-small.train";

  std::ifstream istr(filename);
  if (!istr.is_open())
  {
    std::cerr << "Could not open file " << filename << std::endl;
    return 1;
  }

  FeatureDictionary features("testcpp-features");
  FeatureDictionary labels("labels");
  std::vector<ClassificationExample> examples;
  if (!parseClassificationExamples(istr, features, labels, examples))
    return 1;

  std::cout << examples.size() << " Examples, " << toString(features.getFeatures().count()) << " features, "<< toString(labels.getFeatures().count()) << " labels." << std::endl;

/*  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(
    GradientBasedLearner::createGradientDescent(
      IterationFunction::createConstant(0.01)), labels);
*/
  GradientBasedBinaryClassifierPtr classifier = GradientBasedBinaryClassifier::createLinearSVM(
    GradientBasedLearner::createGradientDescent(
      IterationFunction::createConstant(0.01)), labels);
  
  classifier->createParameters();
  for (int i = 0; i < 100; ++i)
  {
   // LazyVectorPtr gradient = classifier->getEmpiricalRisk(examples)->computeGradient(classifier->getParameters());
//    std::cout << "GRADIENT = " << cralgo::toString(gradient) << std::endl;
    
    //std::cout << "EmpRisk: " << classifier->computeEmpiricalRisk(examples)
    //          << " RegEmpRisk: " << classifier->computeRegularizedEmpiricalRisk(examples) << " ";

    
//  classifier->trainBatch(examples);
    classifier->trainStochastic(examples);

    size_t numCorrect = 0;
    for (size_t i = 0; i < examples.size(); ++i)
      if (classifier->predict(examples[i].getInput()) == examples[i].getOutput())
        ++numCorrect;
    std::cout << "Accuracy: " << (100.0 * numCorrect / (double)examples.size()) << "%" << std::endl;
  }  
  return 0;
}