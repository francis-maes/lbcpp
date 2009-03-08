/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | The list of serializable classes|
| Author  : Francis Maes                   |  objects                        |
| Started : 06/03/2009 19:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/SparseVector.h>
#include <cralgo/DenseVector.h>
#include <cralgo/ContinuousFunction.h>
using namespace cralgo;

//class PerceptronLoss : public StaticToDynamicScalarFunction<impl::PerceptronLoss> {};
//class HingeLoss : public StaticToDynamicScalarFunction<impl::HingeLoss> {};
//class ExponentialLoss : public StaticToDynamicScalarFunction<impl::ExponentialLoss> {};
//class LinearArchitecture : public StaticToDynamicScalarArchitecture<impl::LinearArchitecture> {};

void declareStandardCRAlgoClasses()
{
  DECLARE_CRALGO_CLASS(SparseVector);
  DECLARE_CRALGO_CLASS(DenseVector);

//  DECLARE_CRALGO_CLASS(PerceptronLoss);
//  DECLARE_CRALGO_CLASS(HingeLoss);
}
