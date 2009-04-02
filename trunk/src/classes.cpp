/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | The list of serializable classes|
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lcpp/SparseVector.h>
#include <lcpp/DenseVector.h>
#include <lcpp/ContinuousFunction.h>
using namespace lcpp;

//class PerceptronLoss : public StaticToDynamicScalarFunction<impl::PerceptronLoss> {};
//class HingeLoss : public StaticToDynamicScalarFunction<impl::HingeLoss> {};
//class ExponentialLoss : public StaticToDynamicScalarFunction<impl::ExponentialLoss> {};
//class LinearArchitecture : public StaticToDynamicScalarArchitecture<impl::LinearArchitecture> {};

void declareStandardCRAlgoClasses()
{
  DECLARE_LCPP_CLASS(SparseVector);
  DECLARE_LCPP_CLASS(DenseVector);

//  DECLARE_LCPP_CLASS(PerceptronLoss);
//  DECLARE_LCPP_CLASS(HingeLoss);
}
