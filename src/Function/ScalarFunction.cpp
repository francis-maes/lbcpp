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
size_t ScalarFunction::getNumRequiredInputs() const
  {return 1;}

TypePtr ScalarFunction::getRequiredInputType(size_t index, size_t numInputs) const
  {return doubleType;}

String ScalarFunction::getOutputPostFix() const
  {return T("Scalar");}

TypePtr ScalarFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {return doubleType;}

Variable ScalarFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  double input = inputs[0].getDouble();
  double output = 0.0;
  computeScalarFunction(input, getNumInputs() > 1 ? inputs + 1 : NULL, &output, NULL);
  return output;
}

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
  computeScalarVectorFunction(doubleVector, getNumInputs() > 1 ? inputs + 1 : NULL, &res, NULL, 1.0);
  return res;      
}
