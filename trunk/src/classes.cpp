/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | The list of serializable classes|
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/SparseVector.h>
#include <lbcpp/DenseVector.h>
#include <lbcpp/ContinuousFunction.h>
using namespace lbcpp;

//class PerceptronLoss : public StaticToDynamicScalarFunction<impl::PerceptronLoss> {};
//class HingeLoss : public StaticToDynamicScalarFunction<impl::HingeLoss> {};
//class ExponentialLoss : public StaticToDynamicScalarFunction<impl::ExponentialLoss> {};
//class LinearArchitecture : public StaticToDynamicScalarArchitecture<impl::LinearArchitecture> {};

void declareStandardCRAlgoClasses()
{
  DECLARE_LBCPP_CLASS(SparseVector);
  DECLARE_LBCPP_CLASS(DenseVector);

//  DECLARE_LBCPP_CLASS(PerceptronLoss);
//  DECLARE_LBCPP_CLASS(HingeLoss);
}
