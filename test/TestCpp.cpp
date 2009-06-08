#include <lbcpp/lbcpp.h>
#include <lbcpp/impl/impl.h>
#include <fstream>
using namespace lbcpp;

int testClassification(const std::string& filename)
{
  FeatureDictionaryPtr features = new FeatureDictionary("testcpp-features");
  StringDictionaryPtr labels = new StringDictionary();
  ObjectStreamPtr parser = ObjectStream::createClassificationExamplesParser(filename, features, labels);
  if (!parser)
    return 1;
  ObjectContainerPtr examples = parser->load();

  std::cout << examples->size() << " Examples, " << toString(features->getFeatures()->getNumElements()) << " features, "<< toString(labels->getNumElements()) << " labels." << std::endl;

/*  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(
    GradientBasedLearner::createStochasticDescent(
      IterationFunction::createConstant(0.01)), labels);
*/
//  GradientBasedLearnerPtr learner = GradientBasedLearner::createStochasticDescent(IterationFunction::createConstant(0.01));
  GradientBasedLearnerPtr learner = GradientBasedLearner::createBatch(
     // VectorOptimizer::createGradientDescent(IterationFunction::createConstant(1)),
      VectorOptimizer::createLBFGS(),
    50);
  GradientBasedBinaryClassifierPtr classifier = GradientBasedBinaryClassifier::createLinearSVM(learner, labels);
  
    double acc = classifier->evaluateAccuracy(examples->toStream());
    std::cout << "Initial Accuracy: " << (100.0 * acc) << "%" << std::endl;

  for (int i = 0; i < 1; ++i)
  {
   // FeatureGeneratorPtr gradient = classifier->getEmpiricalRisk(examples)->computeGradient(classifier->getParameters());
//    std::cout << "GRADIENT = " << lbcpp::toString(gradient) << std::endl;
    
    //std::cout << "EmpRisk: " << classifier->computeEmpiricalRisk(examples)
    //          << " RegEmpRisk: " << classifier->computeRegularizedEmpiricalRisk(examples) << " ";

    
    classifier->trainBatch(examples, &ProgressCallback::getConsoleProgressCallback());
    //classifier->trainStochastic(examples);
    double acc = classifier->evaluateAccuracy(examples->toStream());
    std::cout << "Accuracy: " << (100.0 * acc) << "%" << std::endl;
  }
  return 0; 
}

int testRegression(const std::string& filename)
{
  FeatureDictionaryPtr features = new FeatureDictionary("regression-features");
  ObjectStreamPtr parser = ObjectStream::createRegressionExamplesParser(filename, features);
  if (!parser)
    return 1;
  ObjectContainerPtr examples = parser->load();

  std::cout << examples->size() << " Examples, " << toString(features->getFeatures()->getNumElements()) << " features." << std::endl;

  GradientBasedRegressorPtr regressor = GradientBasedRegressor::createLeastSquaresLinear(
      GradientBasedLearner::createStochasticDescent(
      IterationFunction::createConstant(0.01)));
//  regressor->createParameters();
  for (int i = 0; i < 100; ++i)
  {
   // FeatureGeneratorPtr gradient = classifier->getEmpiricalRisk(examples)->computeGradient(classifier->getParameters());
//    std::cout << "GRADIENT = " << lbcpp::toString(gradient) << std::endl;
    
    std::cout << "EmpRisk: " << regressor->computeEmpiricalRisk(examples)
              << " RegEmpRisk: " << regressor->computeRegularizedEmpiricalRisk(examples) << " ";
    regressor->trainStochastic(examples);
    
    double err = regressor->evaluateMeanAbsoluteError(examples->toStream());
    std::cout << "Mean Abs Error: " << err << std::endl;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/binaryclassif/a1a.train";
//  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/regression/pyrim.data";
//  static const char* filename = "/Users/francis/Projets/Francis/data/sequences/NER-small.test";
 // static const char* filename = "/Users/francis/Projets/Francis/data/sequences/NER-small.train";

//  return testRegression(istr);
  return testClassification(filename);
}
