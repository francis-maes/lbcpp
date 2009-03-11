/*-----------------------------------------.---------------------------------.
| Filename: FunctionStatic.hpp             | Continuous Functions            |
| Author  : Francis Maes                   |    Static Interface             |
| Started : 07/03/2009 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_STATIC_INTERFACE_H_
# define CRALGO_IMPL_FUNCTION_STATIC_INTERFACE_H_

# include "BinaryScalarOperation.hpp"

namespace cralgo {
namespace impl {

struct ScalarConstant
{
  ScalarConstant(double value = 0.0)
    : value(value) {}
    
  enum {isDerivable = true};
  
  double compute() const  
    {return value;}

  double value;
};
inline ScalarConstant constant(double value)
  {return ScalarConstant(value);}

template<class ExactType>
struct ContinuousFunction
{
public:
  enum {isDerivable = false};

protected:
  const ExactType& _this() const
    {return *(const ExactType* )this;}
};

template<class ExactType>
struct ScalarFunction : public ContinuousFunction<ExactType>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {assert(false);}
  
protected:
  const ExactType& _this() const
    {return *(const ExactType* )this;}  
};

template<class ExactType>
struct ScalarLossFunction : public ScalarFunction<ExactType>
{
  ScalarLossFunction() {}

  template<class LearningExampleType>
  ScalarLossFunction(const LearningExampleType& learningExample)
    {((ExactType* )this)->setLearningExample(learningExample);}
    
  template<class LearningExampleType>
  void setLearningExample(const LearningExampleType& learningExample)
    {assert(false);}
};

template<class ExactType>
struct ScalarVectorFunction : public ContinuousFunction<ExactType>
{
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {assert(false);}
};

template<class ExactType>
struct VectorLossFunction : public ScalarVectorFunction<ExactType>
{
  VectorLossFunction() {}

  template<class LearningExampleType>
  VectorLossFunction(const LearningExampleType& learningExample)
    {((ExactType* )this)->setLearningExample(learningExample);}
  
  template<class LearningExampleType>
  void setLearningExample(const LearningExampleType& learningExample)
    {assert(false);}
};

template<class ExactType>
struct ScalarArchitecture : public ContinuousFunction<ExactType>
{
  DenseVectorPtr createInitialParameters() const
    {assert(false); return DenseVectorPtr();}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const
    {assert(false);}
  
  
  // todo: non-derivable scalar architectures
};

template<class ExactType>
struct VectorArchitecture : public ContinuousFunction<ExactType>
{
  DenseVectorPtr createInitialParameters() const
    {assert(false); return DenseVectorPtr();}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const
    {assert(false);}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      LazyVectorPtr output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const
    {assert(false);}

  // todo: non-derivable vector architectures
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_STATIC_INTERFACE_H_
