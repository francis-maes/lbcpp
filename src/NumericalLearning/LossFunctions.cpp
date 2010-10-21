/*-----------------------------------------.---------------------------------.
| Filename: LossFunctions.cpp              | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/NumericalLearning/LossFunctions.h>
#include "../Data/Object/DenseDoubleObject.h"
using namespace lbcpp;

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
