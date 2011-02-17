/*-----------------------------------------.---------------------------------.
| Filename: ScalarFunction.cpp             | R -> R Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Function/ScalarFunction.h>
#include <lbcpp/Function/ScalarVectorFunction.h>
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
** ScalarVectorFunction
*/
size_t ScalarVectorFunction::getNumRequiredInputs() const
  {return 1;}

TypePtr ScalarVectorFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return denseDoubleVectorClass();}

String ScalarVectorFunction::getOutputPostFix() const
  {return T("Scalar");}

TypePtr ScalarVectorFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {return doubleType;}

Variable ScalarVectorFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  const DenseDoubleVectorPtr& doubleVector = inputs[0].getObjectAndCast<DenseDoubleVector>();
  jassert(doubleVector);
  double res = 0.0;
  computeScalarVectorFunction(doubleVector, numInputs > 1 ? inputs + 1 : NULL, &res, NULL, 1.0);
  return res;      
}
