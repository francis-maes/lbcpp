/*-----------------------------------------.---------------------------------.
| Filename: FunctionInstantiate.hpp        | Static To Dynamic Functions     |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 19:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_INSTANTIATE_H_
# define CRALGO_IMPL_FUNCTION_INSTANTIATE_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{

/*
** Scalar -> Scalar Functions
*/
template<class ImplementationType, class BaseType>
class StaticToDynamicScalarFunction_ : public BaseType
{
public:
  StaticToDynamicScalarFunction_(const ImplementationType& impl)
    : impl(impl) {}
    
  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual double compute(double input) const
    {double res; impl.compute(input, &res, NULL, NULL); return res;}
  
  virtual double computeDerivative(double input) const
  {
    static const double zero = 0.0;
    assert(ImplementationType::isDerivable);
    double res;
    impl.compute(input, NULL, &zero, &res);
    return res;
  }

  virtual double computeDerivative(double input, double direction) const
    {double res; impl.compute(input, NULL, &direction, &res); return res;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {impl.compute(input, output, derivativeDirection, derivative);}

  virtual void compute(double input, double* output, double* derivative) const
  {
    static const double zero = 0.0;
    impl.compute(input, output, &zero, derivative);
  }

protected:
  ImplementationType impl;
};

template<class ImplementationType>
class StaticToDynamicScalarFunction : public StaticToDynamicScalarFunction_<ImplementationType, ScalarFunction>
{
public:
  StaticToDynamicScalarFunction(const ImplementationType& impl)
    : StaticToDynamicScalarFunction_<ImplementationType, ScalarFunction>(impl) {}
};

template<class ImplementationType>
class StaticToDynamicScalarLossFunction : public StaticToDynamicScalarFunction_<ImplementationType, ScalarLossFunction>
{
public:
  StaticToDynamicScalarLossFunction(const ImplementationType& impl)
    : StaticToDynamicScalarFunction_<ImplementationType, ScalarLossFunction>(impl) {}
    
  virtual void setLearningExample(const LearningExample& learningExample)
    {/*impl.setLearningExample(learningExample);*/} // FIXME => on pert l'info de margin
};

/*
** Vector -> Scalar Functions
*/
template<class ImplementationType, class BaseType>
class StaticToDynamicScalarVectorFunction_ : public BaseType
{
public:
  StaticToDynamicScalarVectorFunction_(const ImplementationType& impl)
    : impl(impl) {}
    
  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual double compute(const FeatureGeneratorPtr input) const
    {assert(input); double res; impl.compute(input, &res, FeatureGeneratorPtr(), LazyVectorPtr()); return res;}
    
  virtual LazyVectorPtr computeGradient(const FeatureGeneratorPtr input) const
  {
    assert(input);
    LazyVectorPtr res(new LazyVector());
    impl.compute(input, NULL, FeatureGeneratorPtr(), res);
    return res;
  }
  
  virtual LazyVectorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const
    {assert(input); LazyVectorPtr res(new LazyVector()); impl.compute(input, NULL, gradientDirection, res); return res;}

  virtual void compute(const FeatureGeneratorPtr input, double* output,
                       const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {assert(input); impl.compute(input, output, gradientDirection, gradient);}
  
  virtual void compute(const FeatureGeneratorPtr input, double* output, LazyVectorPtr gradient) const
  {
    assert(input);
    impl.compute(input, output, FeatureGeneratorPtr(), gradient);
  }
  
private:
  ImplementationType impl;
};

template<class ImplementationType>
class StaticToDynamicScalarVectorFunction : public StaticToDynamicScalarVectorFunction_<ImplementationType, ScalarVectorFunction>
{
public:
  StaticToDynamicScalarVectorFunction(const ImplementationType& impl)
    : StaticToDynamicScalarVectorFunction_<ImplementationType, ScalarVectorFunction>(impl) {}
};

template<class ImplementationType>
class StaticToDynamicVectorLossFunction : public StaticToDynamicScalarVectorFunction_<ImplementationType, VectorLossFunction>
{
public:
  StaticToDynamicVectorLossFunction(const ImplementationType& impl)
    : StaticToDynamicScalarVectorFunction_<ImplementationType, VectorLossFunction>(impl) {}

  virtual void setLearningExample(const LearningExample& learningExample)
    {/*impl.setLearningExample(learningExample);*/} // FIXME => on pert l'info de margin  
};

/*
** Scalar Architecture
*/
template<class ImplementationType>
class StaticToDynamicScalarArchitecture : public ScalarArchitecture
{
public:
  StaticToDynamicScalarArchitecture(const ImplementationType& impl)
    : impl(impl) {}
  
  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const
  {
    assert(parameters && input);
    impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
  }
  
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const
  {
    assert(parameters && input);
    double res;
    impl.compute(parameters, input, &res, LazyVectorPtr(), LazyVectorPtr());
    return res;
  }
  
protected:
  ImplementationType impl;
};


/*
** Vector Architecture
*/
template<class ImplementationType>
class StaticToDynamicVectorArchitecture : public VectorArchitecture
{
public:
  StaticToDynamicVectorArchitecture(const ImplementationType& impl)
    : impl(impl) {}

  virtual bool isDerivable() const
    {return ImplementationType::isDerivable;}

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                        LazyVectorPtr output,
                        LazyVectorPtr gradientWrtParameters,
                        LazyVectorPtr gradientWrtInput) const
  {
    assert(parameters && input);
    impl.compute(parameters, input, output, gradientWrtParameters, gradientWrtInput);
  }

  // todo: non-derivable vector architectures

protected:
  ImplementationType impl;
};

namespace impl
{
#define INSTANTIATE_FUNCTION_(StaticType, DynamicType) \
  template<class ExactType> \
  inline DynamicType##Ptr instantiate(const StaticType <ExactType>& staticFunction) \
    {return DynamicType##Ptr(new StaticToDynamic##DynamicType <ExactType>(static_cast<const ExactType& >(staticFunction)));} 
#define INSTANTIATE_FUNCTION(Type) INSTANTIATE_FUNCTION_(Type, Type)

  INSTANTIATE_FUNCTION(ScalarFunction);
  INSTANTIATE_FUNCTION(ScalarLossFunction);
  INSTANTIATE_FUNCTION(ScalarVectorFunction);
  INSTANTIATE_FUNCTION(VectorLossFunction);
  INSTANTIATE_FUNCTION(ScalarArchitecture);
  INSTANTIATE_FUNCTION(VectorArchitecture);
  
  /*
  template<class ExactType>
  inline ScalarArchitecturePtr instantiate(const ScalarArchitecture<ExactType>& staticArchitecture)
    {return ScalarArchitecturePtr(new StaticToDynamicScalarArchitecture<ExactType>(static_cast<const ExactType& >(staticArchitecture)));}
*/
    
}; /* namespace impl */

}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_STATIC_INTERFACE_H_
