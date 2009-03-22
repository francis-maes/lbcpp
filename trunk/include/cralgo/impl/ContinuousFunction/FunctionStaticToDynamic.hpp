/*-----------------------------------------.---------------------------------.
| Filename: FunctionInstantiate.hpp        | Static To Dynamic Functions     |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_INSTANTIATE_H_
# define CRALGO_IMPL_FUNCTION_INSTANTIATE_H_

# include "FunctionStatic.hpp"
# include "../StaticToDynamic.hpp"

namespace cralgo {
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
    assert(ImplementationType::isDerivable);
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
    {assert(input); double res; BaseClass::impl.compute(input, &res, FeatureGeneratorPtr(), NULL); return res;}
    
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const
  {
    assert(input);
    FeatureGeneratorPtr res;
    BaseClass::impl.compute(input, NULL, FeatureGeneratorPtr(), &res);
    return res;
  }
  
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const
    {assert(input); FeatureGeneratorPtr res; BaseClass::impl.compute(input, NULL, gradientDirection, &res); return res;}

  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const
  {
    assert(input);
    BaseClass::impl.compute(input, output, FeatureGeneratorPtr(), gradient);
  }

  virtual void compute(const FeatureGeneratorPtr input, double* output,
                       const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
    {assert(input); BaseClass::impl.compute(input, output, gradientDirection, gradient);}  
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

  virtual DenseVectorPtr createInitialParameters() const
    {return BaseClass::impl.createInitialParameters();}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(parameters && input);
    BaseClass::impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
  }
  
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const
  {
    assert(parameters && input);
    double res;
    BaseClass::impl.compute(parameters, input, &res, NULL, NULL);
    return res;
  }

STATIC_TO_DYNAMIC_ENDCLASS(ScalarArchitecture);


/*
** Vector Architecture
*/
STATIC_TO_DYNAMIC_CLASS(VectorArchitecture, Object)

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual DenseVectorPtr createInitialParameters() const
    {return BaseClass::impl.createInitialParameters();}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(parameters && input);
    return BaseClass::impl.compute(parameters, input, outputNumber, output, gradientWrtParameters, gradientWrtInput);
  }

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const
  {
    assert(parameters && input);
    double res;
    BaseClass::impl.compute(parameters, input, outputNumber, &res, NULL, NULL);
    return res;
  }

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                        FeatureGeneratorPtr* output,
                        FeatureGeneratorPtr* gradientWrtParameters,
                        FeatureGeneratorPtr* gradientWrtInput) const
  {
    assert(parameters && input);
    BaseClass::impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
  }

  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const
  {
    assert(parameters && input);
    FeatureGeneratorPtr res;
    BaseClass::impl.compute(parameters, input, &res, NULL, NULL);
    return res;
  }

STATIC_TO_DYNAMIC_ENDCLASS(VectorArchitecture);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_STATIC_INTERFACE_H_
