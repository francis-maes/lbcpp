#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
#include <fstream>
using namespace cralgo;

int main(int argc, char* argv[])
{
  static const char* filename = "/Users/francis/Projets/Nieme/trunk/examples/data/binaryclassif/a1a.test";

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

  
  BinaryClassifierPtr classifier = cralgo::createLogisticRegressionClassifier(GradientBasedLearner::createGradientDescent(), labels);
  
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
  
  ScalarArchitecturePtr predictionArchitecture = impl::instantiate(impl::linearArchitecture());
  ScalarLossFunctionPtr lossFunction = impl::instantiate(impl::logBinomialLoss<ClassificationExample>());
  ScalarFunctionPtr lossFunctionWithExample = impl::instantiate(impl::logBinomialLoss(examples[0]));
  ScalarArchitecturePtr lossArchitecture = impl::instantiate(impl::compose(impl::linearArchitecture(), impl::logBinomialLoss(examples[0])));
  
  ScalarVectorFunctionPtr empiricalRisk = impl::instantiate(impl::empiricalRisk(
                            impl::linearArchitecture(),
                            impl::logBinomialLoss<ClassificationExample>(),
                            examples));
  
  ScalarVectorFunctionPtr regularizer = impl::instantiate(impl::sumOfSquares());
  ScalarVectorFunctionPtr weightedRegularizer = impl::instantiate(impl::multiply(impl::sumOfSquares(), impl::constant(0.1)));
  ScalarVectorFunctionPtr regularizedEmpiricalRisk = impl::instantiate(impl::add(
    impl::empiricalRisk(impl::linearArchitecture(), impl::logBinomialLoss<ClassificationExample>(), examples),
    impl::multiply(impl::sumOfSquares(), impl::constant(0.1))));

  DenseVectorPtr parameters(new DenseVector());
  for (int i = 0; i < 100; ++i)
  {
    static const double alpha = 1.0;
    std::cout << "Iteration " << i << " loss = " << empiricalRisk->compute(parameters) << std::endl;
    LazyVectorPtr gradient = empiricalRisk->computeGradient(parameters);
    parameters->addWeighted(gradient, -alpha);
  }
  
  size_t numErrors = 0;
  for (size_t i = 0; i < examples.size(); ++i)
  {
    double activation = predictionArchitecture->compute(parameters, examples[i].getInput());
    bool correct = examples[i].getOutput() > 0;
    bool predicted = activation > 0;
    if (correct != predicted)
      ++numErrors;
  }
  std::cout << numErrors << " Num Errors: " << numErrors << " / " << examples.size() << " => " << (100.0 * numErrors / (double)examples.size()) << "%." << std::endl;


  // ScalarVectorFunctionPtr regularizer = impl::instantiate(impl::multiply(impl::l2norm(), impl::constant(0.1)));
  // ScalarVectorFunctionPtr regularizedRisk = impl::instantiate(impl::add(
  //    impl::empiricalRisk(impl::logBinomialLoss(impl::linearArchitecture()), examples),
  //    impl::multiply(impl::l2norm(), impl::constant(0.1))))
  
  return 0;
}