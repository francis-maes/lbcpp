/*-----------------------------------------.---------------------------------.
| Filename: ScalarFunctions.hpp            | Scalar functions f: R -> R      |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_SCALAR_FUNCTIONS_H_
# define CRALGO_IMPL_FUNCTION_SCALAR_FUNCTIONS_H_

# include "FunctionStatic.hpp"

namespace cralgo {
namespace impl {

// f(x) = x
struct IdentityScalarFunction : public ScalarFunction<IdentityScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output) *output = input;
    if (derivative) *derivative = 1.0;
  }
};
inline IdentityScalarFunction identityScalarFunction()
  {return IdentityScalarFunction();}
  
// f(x) = x * k, k in R
struct MultiplyByConstantScalarFunction : public ScalarFunction<MultiplyByConstantScalarFunction>
{
  MultiplyByConstantScalarFunction(double constant)
    : constant(constant) {}
    
  double constant;
  
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output) *output = input * constant;
    if (derivative) *derivative = constant;
  }
};
inline MultiplyByConstantScalarFunction multiplyScalarByConstant(double constant)
  {return MultiplyByConstantScalarFunction(constant);}

// f(x) = x + k, k in R
struct AddConstantScalarFunction : public ScalarFunction<AddConstantScalarFunction>
{
  AddConstantScalarFunction(double constant)
    : constant(constant) {}
    
  double constant;
  
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    if (output) *output = input + constant;
    if (derivative) *derivative = 1.0;
  }
};
inline AddConstantScalarFunction addConstant(double constant)
  {return AddConstantScalarFunction(constant);}

// f(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
// f(x) in [-1, 1]
struct TanhScalarFunction : public ScalarFunction<TanhScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double ex = exp(input);
    double exm = exp(-input);
    double f = (ex - exm) / (ex + exm);
    if (output)
      *output = f;
    if (derivative)
      *derivative = 1 - f * f;
  }
};
inline TanhScalarFunction tanhFunction()
  {return TanhScalarFunction();}


// f(x) = 2.0 / (1.0 + exp(-x)) - 1
// f(x) in [-1, 1]
struct SigmoidScalarFunction : public ScalarFunction<SigmoidScalarFunction>
{
  enum {isDerivable = true};

  void compute(double input, double* output, const double* , double* derivative) const
  {
    double s = 1.0 / (1.0 + exp(-input));
    double ss = 2 * s;
    if (output)
      *output = ss - 1;
    if (derivative)
      *derivative = ss * (1 - s);
  }
};
inline SigmoidScalarFunction sigmoidFunction()
  {return SigmoidScalarFunction();}

template<class VectorScalarFunctionType>
struct VectorLineScalarFunction
  : public ScalarFunction<VectorLineScalarFunction< VectorScalarFunctionType > >
{
  VectorLineScalarFunction(const VectorScalarFunctionType& function, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
    : function(function), parameters(parameters), direction(direction) {}
    
  enum {isDerivable = VectorScalarFunctionType::isDerivable};
  
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    DenseVectorPtr vectorInput = new DenseVector(parameters->getDictionary());
    vectorInput->add(parameters);
    vectorInput->addWeighted(direction, input);
    FeatureGeneratorPtr gradientDirection, gradient;
    if (derivativeDirection)
      gradientDirection = FeatureGenerator::multiplyByScalar(direction, *derivativeDirection > 0 ? 1.0 : -1.0);
    function.compute(vectorInput, output, gradientDirection, derivative ? &gradient : NULL);
    if (derivative)
      *derivative = direction->dotProduct(gradient);
  }
  
private:
  VectorScalarFunctionType function;
  const FeatureGeneratorPtr parameters;
  const FeatureGeneratorPtr direction;
  
  DenseVectorPtr input;
};

template<class ExactType>
inline VectorLineScalarFunction<ExactType> vectorLineScalarFunction(const ScalarVectorFunction<ExactType>& function,
          const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
  {return VectorLineScalarFunction<ExactType>(static_cast<const ExactType& >(function), parameters, direction);}

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_SCALAR_FUNCTIONS_H_
