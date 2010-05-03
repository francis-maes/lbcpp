/*-----------------------------------------.---------------------------------.
| Filename: FunctionStatic.hpp             | Continuous Functions            |
| Author  : Francis Maes                   |    Static Interface             |
| Started : 07/03/2009 14:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_STATIC_INTERFACE_H_
# define LBCPP_CORE_IMPL_FUNCTION_STATIC_INTERFACE_H_

# include "BinaryScalarOperation.hpp"
# include "../Object.hpp"
# include "../../ContinuousFunction.h"
# include <cfloat>

namespace lbcpp {
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
struct ContinuousFunction : public Object<ExactType>
{
public:
  enum {isDerivable = false};
};

template<class ExactType>
struct ScalarFunction : public ContinuousFunction<ExactType>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {jassert(false);}
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
    {jassert(false);}
};

template<class ExactType>
struct ScalarVectorFunction : public ContinuousFunction<ExactType>
{
  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
    {jassert(false);}
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
    {jassert(false);}
};

template<class ExactType>
struct ScalarArchitecture : public ContinuousFunction<ExactType>
{
  ScalarArchitecture() : dotProductCache(NULL) {}

  FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {jassert(false); return FeatureDictionaryPtr();}
    
  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
    {jassert(false);}
  
  FeatureGenerator::DotProductCache* dotProductCache;
  
  // todo: non-derivable scalar architectures
};

template<class ExactType>
struct VectorArchitecture : public ContinuousFunction<ExactType>
{
  FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {jassert(false); return FeatureDictionaryPtr();}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
    {jassert(false);}

  void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      FeatureGeneratorPtr* output,
      FeatureGeneratorPtr* gradientsWrtParameters,
      FeatureGeneratorPtr* gradientsWrtInput) const
    {jassert(false);}

  // todo: non-derivable vector architectures
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_STATIC_INTERFACE_H_
