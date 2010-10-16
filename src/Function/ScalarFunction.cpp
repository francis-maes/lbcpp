/*-----------------------------------------.---------------------------------.
| Filename: ScalarFunction.cpp             | R -> R Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarFunction.h>
#include <lbcpp/Function/ScalarObjectFunction.h>
using namespace lbcpp;

/*
** Scalar Function
*/
double ScalarFunction::compute(double input) const
{
  double res;
  compute(input, &res, NULL, NULL);
  return res;
}

double ScalarFunction::computeDerivative(double input) const
{
  double res;
  compute(input, NULL, NULL, &res);
  return res;
}

double ScalarFunction::computeDerivative(double input, double direction) const
{
  double res;
  compute(input, NULL, &direction, &res);
  return res;
}

void ScalarFunction::compute(double input, double* output, double* derivative) const
  {compute(input, output, NULL, derivative);}

ScalarFunctionPtr ScalarFunction::multiplyByScalar(double scalar)
  {return multiplyByScalarFunction(refCountedPointerFromThis(this), scalar);}

ScalarFunctionPtr ScalarFunction::composeWith(ScalarFunctionPtr postFunction) const
  {return composeScalarFunction(refCountedPointerFromThis(this), postFunction);}

/*
** BinaryClassificationLossFunction
*/
String BinaryClassificationLossFunction::toString() const
  {return getClassName() + T("(") + (isPositive ? T("+") : T("-")) + T(")");}

void BinaryClassificationLossFunction::compute(double input, double* output, const double* derivativeDirection, double* derivative) const
{
  double dd;
  if (derivativeDirection)
    dd = isPositive ? *derivativeDirection : -(*derivativeDirection);
  computePositive(isPositive ? input : -input, output, derivativeDirection ? &dd : NULL, derivative);
  if (derivative && !isPositive)
    *derivative = - (*derivative);
}

////////////////
# include "../Data/Object/DenseDoubleObject.h"

/*
** ScalarObjectFunction
*/
ScalarObjectFunctionPtr ScalarObjectFunction::multiplyByScalar(double weight) const
  {return multiplyByScalarObjectFunction(refCountedPointerFromThis(this), weight);}

/*
** MultiClassLossFunction
*/
MultiClassLossFunction::MultiClassLossFunction(EnumerationPtr classes, size_t correctClass)
  : classes(classes), correctClass(correctClass) {}

String MultiClassLossFunction::toString() const
  {return getClassName() + T("(") + String((int)correctClass) + T(")");}

void MultiClassLossFunction::compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const
{
  DenseDoubleObjectPtr denseDoubleInput = input.dynamicCast<DenseDoubleObject>();
  if (!input || denseDoubleInput)
  {
    jassert(!gradientTarget || !*gradientTarget || gradientTarget->isInstanceOf<DenseDoubleObject>());
    std::vector<double>* gradientVectorTarget = NULL;
    if (gradientTarget)
    {
      DenseDoubleObjectPtr denseGradientTarget;
      if (*gradientTarget)
        denseGradientTarget = gradientTarget->dynamicCast<DenseDoubleObject>();
      else
      {
        denseGradientTarget = new DenseDoubleObject(enumBasedDoubleVectorClass(classes), 0.0);
        *gradientTarget = denseGradientTarget;
      }
      jassert(denseGradientTarget);
      gradientVectorTarget = &denseGradientTarget->getValues();
      jassert(gradientVectorTarget->size() == classes->getNumElements());
    }

    compute(denseDoubleInput ? &denseDoubleInput->getValues() : NULL, output, gradientVectorTarget, gradientWeight);
  }
  else
    jassert(false); // not implemented
}
