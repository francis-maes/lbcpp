/*-----------------------------------------.---------------------------------.
| Filename: ScalarVectorDerivableFunc...hpp| Derivable functions f: R^d -> R |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

// f(x) = sum_i x_i^2
struct SumOfSquaresScalarVectorFunction : public ScalarVectorFunction< SumOfSquaresScalarVectorFunction >
{
  enum {isDerivable = true};
  
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, LazyVectorPtr gradient) const
  {
    if (output)
      *output = input->sumOfSquares();
    if (gradient)
      gradient->addWeighted(input, 2.0);
  }
};

inline SumOfSquaresScalarVectorFunction sumOfSquares()
  {return SumOfSquaresScalarVectorFunction();}


struct MultiClassLogBinomialLossFunction : public ScalarVectorFunction< MultiClassLogBinomialLossFunction >
{
  MultiClassLogBinomialLossFunction() : correctClass(-1) {}
  
  enum {isDerivable = true};
    
  void setCorrectClass(size_t correctClass)
    {this->correctClass = (int)correctClass;}
    
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr, LazyVectorPtr gradient) const
  {
    assert(correctClass >= 0);
    DenseVectorPtr scores = input->toDenseVector();
    assert(scores);
    
    double logZ = scores->computeLogSumOfExponentials();
    if (!isNumberValid(logZ))
    {
      std::cerr << "LogZ is not a valid number. Scores: " << toString(scores) << std::endl;
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
      gradient->set(res);
    }
  }
  
private:
  int correctClass;
};

inline MultiClassLogBinomialLossFunction multiClassLogBinomialLossFunction()
  {return MultiClassLogBinomialLossFunction();}

// todo: ranking losses

}; /* namespace impl */
}; /* namespace cralgo */


#endif // !CRALGO_IMPL_FUNCTION_SCALAR_VALUED_VECTOR_DERIVABLE_H_
