#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
#include <fstream>
using namespace cralgo;

int testClassification(std::istream& istr)
{
  FeatureDictionaryPtr features = new FeatureDictionary("testcpp-features");
  StringDictionaryPtr labels = new StringDictionary();

  std::vector<ClassificationExample> examples;
  if (!parseClassificationExamples(istr, features, labels, examples))
    return 1;

  std::cout << examples.size() << " Examples, " << toString(features->getFeatures()->getNumElements()) << " features, "<< toString(labels->getNumElements()) << " labels." << std::endl;

/*  GradientBasedClassifierPtr classifier = GradientBasedClassifier::createMaximumEntropy(
    GradientBasedLearner::createStochasticDescent(
      IterationFunction::createConstant(0.01)), labels);
*/
//  GradientBasedLearnerPtr learner = GradientBasedLearner::createStochasticDescent(IterationFunction::createConstant(0.01));
  GradientBasedLearnerPtr learner = GradientBasedLearner::createBatch(VectorOptimizer::createStochasticDescent(IterationFunction::createConstant(1)), OptimizerTerminationTest::createMaxIterations(50));
  GradientBasedBinaryClassifierPtr classifier = GradientBasedBinaryClassifier::createLinearSVM(learner, labels);
  
  for (int i = 0; i < 1; ++i)
  {
   // FeatureGeneratorPtr gradient = classifier->getEmpiricalRisk(examples)->computeGradient(classifier->getParameters());
//    std::cout << "GRADIENT = " << cralgo::toString(gradient) << std::endl;
    
    //std::cout << "EmpRisk: " << classifier->computeEmpiricalRisk(examples)
    //          << " RegEmpRisk: " << classifier->computeRegularizedEmpiricalRisk(examples) << " ";

    
    classifier->trainBatch(examples);
    //classifier->trainStochastic(examples);
    double acc = classifier->evaluateAccuracy(examples);
    std::cout << "Accuracy: " << (100.0 * acc) << "%" << std::endl;
  }
  return 0; 
}

int testRegression(std::istream& istr)
{
  FeatureDictionaryPtr features = new FeatureDictionary("regression-features");
  std::vector<RegressionExample> examples;
  if (!parseRegressionExamples(istr, features, examples))
    return 1;

  std::cout << examples.size() << " Examples, " << toString(features->getFeatures()->getNumElements()) << " features." << std::endl;

  GradientBasedRegressorPtr regressor = GradientBasedRegressor::createLeastSquaresLinear(
      GradientBasedLearner::createStochasticDescent(
      IterationFunction::createConstant(0.01)));
  regressor->createParameters();
  for (int i = 0; i < 100; ++i)
  {
   // FeatureGeneratorPtr gradient = classifier->getEmpiricalRisk(examples)->computeGradient(classifier->getParameters());
//    std::cout << "GRADIENT = " << cralgo::toString(gradient) << std::endl;
    
    std::cout << "EmpRisk: " << regressor->computeEmpiricalRisk(examples)
              << " RegEmpRisk: " << regressor->computeRegularizedEmpiricalRisk(examples) << " ";
    regressor->trainStochastic(examples);
    
    double err = regressor->evaluateMeanAbsoluteError(examples);
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

  std::ifstream istr(filename);
  if (!istr.is_open())
  {
    std::cerr << "Could not open file " << filename << std::endl;
    return 1;
  }

//  return testRegression(istr);
  return testClassification(istr);
}
