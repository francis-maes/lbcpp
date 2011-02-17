/*-----------------------------------------.---------------------------------.
| Filename: LossFunction.cpp               | Learning Loss Functions         |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 19:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Learning/LossFunction.h>
using namespace lbcpp;

/*
** RegressionLossFunction
*/
size_t RegressionLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr RegressionLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return doubleType;}

String RegressionLossFunction::getOutputPostFix() const
  {return T("Loss");}

void RegressionLossFunction::computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const
  {computeRegressionLoss(input, otherInputs[0].getDouble(), output, derivative);}

/*
** DiscriminativeLossFunction
*/
size_t DiscriminativeLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr DiscriminativeLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? booleanType : doubleType;}

String DiscriminativeLossFunction::getOutputPostFix() const
  {return T("Loss");}

void DiscriminativeLossFunction::computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const
{
  bool isPositive = otherInputs[0].getBoolean();
  computeDiscriminativeLoss(isPositive ? input : -input, output, derivative);
  if (derivative && !isPositive)
    *derivative = - (*derivative);
}

/*
** MultiClassLossFunction
*/
size_t MultiClassLossFunction::getNumRequiredInputs() const
  {return 2;}

TypePtr MultiClassLossFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return index == 1 ? enumValueType : (TypePtr)denseDoubleVectorClass();}

TypePtr MultiClassLossFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  classes = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
  jassert(classes);
  if (classes != inputVariables[1]->getType())
  {
    context.errorCallback(T("Type mismatch: double vector type is ") + classes->getName() + T(" supervision type is ") + inputVariables[1]->getType()->getName());
    return TypePtr();
  }
  return ScalarVectorFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
}

void MultiClassLossFunction::computeScalarVectorFunction(const DenseDoubleVectorPtr& scores, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
{
  int correct = otherInputs[0].getInteger();
  size_t numClasses = classes->getNumElements();
  jassert(numClasses > 1);
  jassert(correct >= 0 && correct < (int)numClasses);
  computeMultiClassLoss(scores, (size_t)correct, numClasses, output, gradientTarget, gradientWeight);
}
