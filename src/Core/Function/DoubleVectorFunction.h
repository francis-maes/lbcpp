/*-----------------------------------------.---------------------------------.
| Filename: DoubleVectorFunction.h         | Function related to             |
| Author  : Julien Becker                  |                    DoubleVector |
| Started : 06/10/2011 09:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_FUNCTION_DOUBLE_VECTOR_H_
# define LBCPP_CORE_FUNCTION_DOUBLE_VECTOR_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Data/RandomVariable.h>

namespace lbcpp
{

/*
** Double Vector Entropy Function
*/

class DoubleVectorEntropyFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, probabilityType);}

  virtual String getOutputPostFix() const
    {return T("Entropy");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return negativeLogProbabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& distribution = inputs[0].getObjectAndCast<DoubleVector>();
    if (distribution)
      return Variable(distribution->entropy(), negativeLogProbabilityType);
    else
      return Variable::missingValue(negativeLogProbabilityType);
  }
};

/*
** Double Vector Normalize Function
*/

class DoubleVectorNormalizeBatchLearner;

class DoubleVectorNormalizeFunction : public Function
{
public:
  DoubleVectorNormalizeFunction(bool useVariances = true, bool useMeans = false)
    {setBatchLearner(doubleVectorNormalizeBatchLearner(useVariances, useMeans));}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, doubleType);}

  virtual String getOutputPostFix() const
    {return T("Normalize");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("dvNorm");
    outputShortName = T("dvNorm");
    return denseDoubleVectorClass(inputVariables[0]->getType()->getTemplateArgument(0), inputVariables[0]->getType()->getTemplateArgument(1));
  }
protected:
  friend class DoubleVectorNormalizeFunctionClass;
  friend class DoubleVectorNormalizeBatchLearner;

  std::vector<double> variances;
  std::vector<double> means;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DenseDoubleVectorPtr res = input.getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    const size_t n = res->getNumValues();
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, (res->getValue(i) - means[i]) / variances[i]);
    return res;
  }
};

extern ClassPtr doubleVectorNormalizeFunctionClass;
typedef ReferenceCountedObjectPtr<DoubleVectorNormalizeFunction> DoubleVectorNormalizeFunctionPtr;

/*
** Concatenated Double Vector Normalize Function
*/
class ConcatenatedDoubleVectorNormalizeBatchLearner;

class ConcatenatedDoubleVectorNormalizeFunction : public Function
{
public:
  ConcatenatedDoubleVectorNormalizeFunction()
    {setBatchLearner(concatenatedDoubleVectorNormalizeBatchLearner());}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, doubleType);}
  
  virtual String getOutputPostFix() const
    {return T("ConcatenatedNormalize");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("cdvNorm");
    outputShortName = T("cdvNorm");
    return denseDoubleVectorClass(inputVariables[0]->getType()->getTemplateArgument(0), inputVariables[0]->getType()->getTemplateArgument(1));
  }
protected:
  friend class ConcatenatedDoubleVectorNormalizeFunctionClass;
  friend class ConcatenatedDoubleVectorNormalizeBatchLearner;

  std::vector<double> zFactors;

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DenseDoubleVectorPtr res = input.getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
    const size_t n = res->getNumValues();
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, res->getValue(i) / zFactors[i]);
    return res;
  }
};

extern ClassPtr concatenatedDoubleVectorNormalizeFunctionClass;
typedef ReferenceCountedObjectPtr<ConcatenatedDoubleVectorNormalizeFunction> ConcatenatedDoubleVectorNormalizeFunctionPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_FUNCTION_DOUBLE_VECTOR_H_
