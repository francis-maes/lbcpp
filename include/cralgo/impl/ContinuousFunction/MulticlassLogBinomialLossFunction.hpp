/*-----------------------------------------.---------------------------------.
| Filename: MulticlassLogBinomialLoss...hpp| Multi-class log-binomial loss   |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_MULTICLASS_LOG_BINOMIAL_LOSS_FUNCTION_H_
# define CRALGO_IMPL_FUNCTION_MULTICLASS_LOG_BINOMIAL_LOSS_FUNCTION_H_

# include "LossFunctions.hpp"

namespace cralgo {
namespace impl {

struct MultiClassLogBinomialLossFunction : public ScalarVectorFunction< MultiClassLogBinomialLossFunction >
{
  MultiClassLogBinomialLossFunction() : correctClass(-1) {}
  
  enum {isDerivable = true};
    
  void setCorrectClass(size_t correctClass)
    {this->correctClass = (int)correctClass;}
    
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, FeatureGeneratorPtr* gradient) const
  {
    assert(correctClass >= 0);
    DenseVectorPtr scores = input->toDenseVector();
    if (!scores || !scores->getNumValues())
    {
      std::cerr << "No Scores, input = " << input->getClassName() << " inputDictionary = " << input->getDictionary()->getName() << "inputToString = " << input->toString() << std::endl;
      assert(false);
    }
    assert(scores && scores->getNumValues());
    
    double logZ = scores->computeLogSumOfExponentials();
    if (!isNumberValid(logZ))
    {
      std::cerr << "LogZ is not a valid number. Scores: " << cralgo::toString(scores) << std::endl;
      assert(false);
    }
    if (output)
      *output = logZ - scores->get(correctClass);
    if (gradient)
    {
      DenseVectorPtr res = new DenseVector(scores->getDictionary(), scores->getNumValues());
      for (size_t i = 0; i < scores->getNumValues(); ++i)
      {
        double derivative = exp(scores->get(i) - logZ);
        assert(isNumberValid(derivative) && isNumberValid(derivative));
        res->set(i, derivative);
      }
      res->get(correctClass) -= 1.0;
      *gradient = res;
    }
  }
  
private:
  int correctClass;
};

inline MultiClassLogBinomialLossFunction multiClassLogBinomialLossFunction()
  {return MultiClassLogBinomialLossFunction();}

STATIC_MULTICLASS_LOSS_FUNCTION(multiClassLogBinomialLoss, MultiClassLogBinomialLoss, MultiClassLogBinomialLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_FUNCTION_MULTICLASS_LOG_BINOMIAL_LOSS_FUNCTION_H_
