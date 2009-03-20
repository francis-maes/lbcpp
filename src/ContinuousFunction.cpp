/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.cpp         | Continuous Functions            |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/ContinuousFunction.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

class VectorFunctionLineScalarFunction
  : public impl::StaticToDynamicScalarFunction< impl::VectorLineScalarFunction< impl::DynamicToStaticScalarVectorFunction > >
{
public:
  typedef impl::StaticToDynamicScalarFunction< impl::VectorLineScalarFunction< impl::DynamicToStaticScalarVectorFunction > > BaseClass;
  
  VectorFunctionLineScalarFunction(ScalarVectorFunctionPtr function, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
    : BaseClass(impl::vectorLineScalarFunction(impl::dynamicToStatic(function), parameters, direction)) {}
};

ScalarFunctionPtr ScalarFunction::createVectorFunctionLine(ScalarVectorFunctionPtr function, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
{
  return new VectorFunctionLineScalarFunction(function, parameters, direction);
}

