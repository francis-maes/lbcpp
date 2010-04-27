/*-----------------------------------------.---------------------------------.
| Filename: FunctionInstantiate.hpp        | Static To Dynamic Functions     |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_IMPL_FUNCTION_INSTANTIATE_H_
# define LBCPP_CORE_IMPL_FUNCTION_INSTANTIATE_H_

# include "FunctionStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace lbcpp {
namespace impl {

/*
** Scalar -> Scalar Functions
*/
STATIC_TO_DYNAMIC_ABSTRACT_CLASS(ScalarFunction_, Object)

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual double compute(double input) const
    {double res; BaseClass::impl.compute(input, &res, NULL, NULL); return res;}
  
  virtual double computeDerivative(double input) const
  {
    static const double zero = 0.0;
    jassert(ImplementationType::isDerivable);
    double res;
    BaseClass::impl.compute(input, NULL, &zero, &res);
    return res;
  }

  virtual double computeDerivative(double input, double direction) const
    {double res; BaseClass::impl.compute(input, NULL, &direction, &res); return res;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {BaseClass::impl.compute(input, output, derivativeDirection, derivative);}

  virtual void compute(double input, double* output, double* derivative) const
  {
    static const double zero = 0.0;
    BaseClass::impl.compute(input, output, &zero, derivative);
  }
};

STATIC_TO_DYNAMIC_CLASS(ScalarFunction, ScalarFunction_)
STATIC_TO_DYNAMIC_ENDCLASS(ScalarFunction);


STATIC_TO_DYNAMIC_CLASS(ScalarLossFunction, ScalarFunction_)
  virtual void setLearningExample(const LearningExample& learningExample)
    {/*impl.setLearningExample(learningExample);*/} // FIXME => on pert l'info de margin
STATIC_TO_DYNAMIC_ENDCLASS(ScalarLossFunction);

/*
** Vector -> Scalar Functions
*/
STATIC_TO_DYNAMIC_ABSTRACT_CLASS(ScalarVectorFunction_, Object)

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual double compute(const FeatureGeneratorPtr input) const
    {jassert(input); double res; BaseClass::impl.compute(input, &res, FeatureGeneratorPtr(), NULL); return res;}
    
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const
  {
    jassert(input);
    FeatureGeneratorPtr res;
    BaseClass::impl.compute(input, NULL, FeatureGeneratorPtr(), &res);
    return res;
  }
  
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const
  {
    jassert(input);
    FeatureGeneratorPtr res;
    BaseClass::impl.compute(input, NULL, gradientDirection, &res);
    jassert(res->getDictionary() == input->getDictionary());
    return res;
  }

  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const
  {
    jassert(input);
    BaseClass::impl.compute(input, output, FeatureGeneratorPtr(), gradient);
    jassert(!gradient || (*gradient)->getDictionary() == input->getDictionary());
  }

  virtual void compute(const FeatureGeneratorPtr input, double* output,
                       const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    jassert(input);
    BaseClass::impl.compute(input, output, gradientDirection, gradient);
    if (gradient)
      (*gradient)->checkDictionaryEquals(input->getDictionary());
  }
};

STATIC_TO_DYNAMIC_CLASS(ScalarVectorFunction, ScalarVectorFunction_)
STATIC_TO_DYNAMIC_ENDCLASS(ScalarVectorFunction);

STATIC_TO_DYNAMIC_CLASS(VectorLossFunction, ScalarVectorFunction_)

  virtual void setLearningExample(const LearningExample& learningExample)
    {/*impl.setLearningExample(learningExample);*/} // FIXME => on pert l'info de margin  

STATIC_TO_DYNAMIC_ENDCLASS(VectorLossFunction);

/*
** Scalar Architecture
*/
STATIC_TO_DYNAMIC_CLASS(ScalarArchitecture, Object)

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {return BaseClass::impl.getParametersDictionary(inputDictionary);}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
  {
    jassert(parameters && input);
    BaseClass::impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
    jassert(!gradientWrtParameters || (*gradientWrtParameters)->getDictionary() == parameters->getDictionary());
    jassert(!gradientWrtInput || (*gradientWrtInput)->getDictionary() == input->getDictionary());
  }
  
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const
  {
    if (parameters && input)
    {
      double res;
      BaseClass::impl.compute(parameters, input, &res, NULL, NULL);
      return res;
    }
    else
      return 0.0;
  }

STATIC_TO_DYNAMIC_ENDCLASS(ScalarArchitecture);


/*
** Vector Architecture
*/
STATIC_TO_DYNAMIC_CLASS(VectorArchitecture, Object)

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const
    {return BaseClass::impl.getParametersDictionary(inputDictionary);}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    jassert(parameters && input);
    BaseClass::impl.compute(parameters, input, outputNumber, output, gradientWrtParameters, gradientWrtInput);
    jassert(!gradientWrtParameters || (*gradientWrtParameters)->getDictionary() == parameters->getDictionary());
    jassert(!gradientWrtInput || (*gradientWrtInput)->getDictionary() == input->getDictionary());
  }

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const
  {
    jassert(parameters && input);
    double res;
    BaseClass::impl.compute(parameters, input, outputNumber, &res, NULL, NULL);
    return res;
  }

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                        FeatureGeneratorPtr* output,
                        FeatureGeneratorPtr* gradientWrtParameters,
                        FeatureGeneratorPtr* gradientWrtInput) const
  {
    jassert(parameters && input);
    BaseClass::impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
    // todo: check dictionary
  }

  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const
  {
    jassert(parameters && input);
    FeatureGeneratorPtr res;
    BaseClass::impl.compute(parameters, input, &res, NULL, NULL);
    return res;
  }

STATIC_TO_DYNAMIC_ENDCLASS(VectorArchitecture);

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_CORE_IMPL_FUNCTION_STATIC_INTERFACE_H_
