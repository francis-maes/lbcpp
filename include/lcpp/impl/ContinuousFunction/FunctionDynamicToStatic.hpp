/*-----------------------------------------.---------------------------------.
| Filename: FunctionDynamicToStatic.hpp    | Continuous function             |
| Author  : Francis Maes                   |       dynamic -> static bridge  |
| Started : 20/03/2009 16:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_CONTINUOUS_FUNCTION_DYNAMIC_TO_STATIC_H_
# define LCPP_CORE_IMPL_CONTINUOUS_FUNCTION_DYNAMIC_TO_STATIC_H_

# include "FunctionStatic.hpp"

namespace lcpp {
namespace impl {

struct DynamicToStaticScalarFunction : public ScalarFunction<DynamicToStaticScalarFunction>
{
  DynamicToStaticScalarFunction(ScalarFunctionPtr function)
    : function(function) {}
  
  ScalarFunctionPtr function;

  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
    {
      if (derivative)
      {
        if (derivativeDirection)
          function->compute(input, output, derivativeDirection, derivative);
        else
          function->compute(input, output, derivative);
      }
      else
        *output = function->compute(input);
    }
    else if (derivative)
    {
      if (derivativeDirection)
        *derivative = function->computeDerivative(input, *derivativeDirection);
      else
        *derivative = function->computeDerivative(input);
    }
  }
};

inline DynamicToStaticScalarFunction dynamicToStatic(ScalarFunctionPtr function)
  {return DynamicToStaticScalarFunction(function);}

struct DynamicToStaticScalarVectorFunction : public ScalarVectorFunction<DynamicToStaticScalarVectorFunction>
{
  DynamicToStaticScalarVectorFunction(ScalarVectorFunctionPtr function)
    : function(function) {}
  
  ScalarVectorFunctionPtr function;

  void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const
  {
    if (output)
    {
      if (gradient)
      {
        if (gradientDirection)
          function->compute(input, output, gradientDirection, gradient);
        else
          function->compute(input, output, gradient);
      }
      else
        *output = function->compute(input);
    }
    else if (gradient)
    {
      if (gradientDirection)
        *gradient = function->computeGradient(input, gradientDirection);
      else
        *gradient = function->computeGradient(input);
    }
  }
};

inline DynamicToStaticScalarVectorFunction dynamicToStatic(ScalarVectorFunctionPtr function)
  {return DynamicToStaticScalarVectorFunction(function);}

}; /* namespace impl */
}; /* namespace lcpp */

#endif // !LCPP_CORE_IMPL_CONTINUOUS_FUNCTION_DYNAMIC_TO_STATIC_H_
