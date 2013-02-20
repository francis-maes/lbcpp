/*-----------------------------------------.---------------------------------.
| Filename: DimericCompositionFunction.h   | Dimeric Composition Function    |
| Author  : Julien Becker                  |                                 |
| Started : 20/02/2013 20:32               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DIMERIC_COMPOSITION_FUNCTION_H_
# define LBCPP_PROTEIN_DIMERIC_COMPOSITION_FUNCTION_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class DimericCompositionFunction : public Function
{
public:
  virtual String getOutputPostFix() const
    {return T("Dimer");}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(getInputElementsType());}

  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    EnumerationPtr outputEnum = getOutputEnumeration(inputVariables[0]->getType());
    numEnumerationElements = outputEnum->getNumElements();
    return doubleVectorClass(cartesianProductEnumerationEnumeration(outputEnum, outputEnum), probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const VectorPtr& inputVector = input.getObjectAndCast<Vector>(context);
    if (!inputVector)
      return Variable::missingValue(getOutputType());

    const size_t numElements = inputVector->getNumElements();
    const double incrementValue = 1.f / (float)(numElements - 1.f);

    SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getOutputType());
    int previousValue = -1;
    for (size_t i = 0; i < numElements; ++i)
    {
      int value = getValue(inputVector->getElement(i));
      if (value >= 0 && previousValue >= 0)
        res->incrementValue(previousValue * numEnumerationElements + value, incrementValue);
      previousValue = value;
    }
    return res;
  }

protected:
  friend class DimericCompositionFunctionClass;

  size_t numEnumerationElements;

  virtual TypePtr getInputElementsType() const = 0;
  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType) = 0;
  virtual int getValue(const Variable& v) const = 0;
};

class AminoAcidDimericCompositionFunction : public DimericCompositionFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return aminoAcidTypeEnumeration;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return standardAminoAcidTypeEnumeration;}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue() || v.getInteger() >= (int)numEnumerationElements)
      return -1;
    return v.getInteger();
  }
};

class DoubleVectorDimericCompositionFunction : public DimericCompositionFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return doubleVectorClass(enumerationClass, doubleType);}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return inputType->getTemplateArgument(0);}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return -1;
    return v.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
  }
};

class ProbabilityDimericCompositionFunction : public DimericCompositionFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return probabilityType;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return falseOrTrueEnumeration;}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return -1;
    return v.getDouble() >= 0.5 ? 1 : 0;
  }
};

class DimericCompositionProxyFunction : public ProxyFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual String getOutputPostFix() const
    {return T("DimerProxy");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(anyType);}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputType = inputVariables[0]->getType();

    if (inputType == probabilityType)
      return new ProbabilityDimericCompositionFunction();
    else if (inputType == aminoAcidTypeEnumeration)
      return new AminoAcidDimericCompositionFunction();
    else if (inputType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return new DoubleVectorDimericCompositionFunction();
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }
};

class LocalDimericCompositionFunction : public Function
{
public:
  LocalDimericCompositionFunction(size_t windowSize)
    : windowSize(windowSize) {}

  virtual String getOutputPostFix() const
    {return T("LocalDimer");}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)vectorClass(getInputElementsType()) : positiveIntegerType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    EnumerationPtr outputEnum = getOutputEnumeration(inputVariables[0]->getType());
    numEnumerationElements = outputEnum->getNumElements();
    return doubleVectorClass(cartesianProductEnumerationEnumeration(outputEnum, outputEnum), probabilityType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const VectorPtr& inputVector = inputs[0].getObjectAndCast<Vector>(context);
    if (!inputVector)
      return Variable::missingValue(getOutputType());

    const size_t position = inputs[1].getInteger();
    const size_t numElements = inputVector->getNumElements();
    const double incrementValue = 1.f / (float)(windowSize - 1.f);

    SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getOutputType());
    int previousValue = -1;
    size_t firstIndex = (size_t)juce::jmax((int)(position - windowSize / 2), 0);
    size_t lastIndex = (size_t)juce::jmin((int)(position + windowSize / 2), (int)numElements - 1);
    for (size_t i = firstIndex; i <= lastIndex; ++i)
    {
      int value = getValue(inputVector->getElement(i));
      if (value >= 0 && previousValue >= 0)
        res->incrementValue(previousValue * numEnumerationElements + value, incrementValue);
      previousValue = value;      
    }
    return res;
  }

protected:
  friend class LocalDimericCompositionFunctionClass;

  size_t windowSize;  
  size_t numEnumerationElements;

  LocalDimericCompositionFunction() {}

  virtual TypePtr getInputElementsType() const = 0;
  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType) = 0;
  virtual int getValue(const Variable& v) const = 0;
};

class AminoAcidLocalDimericCompositionFunction : public LocalDimericCompositionFunction
{
public:
  AminoAcidLocalDimericCompositionFunction(size_t windowSize)
    : LocalDimericCompositionFunction(windowSize) {}

protected:
  virtual TypePtr getInputElementsType() const
    {return aminoAcidTypeEnumeration;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return standardAminoAcidTypeEnumeration;}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue() || v.getInteger() >= (int)numEnumerationElements)
      return -1;
    return v.getInteger();
  }

protected:
  friend class AminoAcidLocalDimericCompositionFunctionClass;

  AminoAcidLocalDimericCompositionFunction() {}
};

class DoubleVectorLocalDimericCompositionFunction : public LocalDimericCompositionFunction
{
public:
  DoubleVectorLocalDimericCompositionFunction(size_t windowSize)
    : LocalDimericCompositionFunction(windowSize) {}

protected:
  virtual TypePtr getInputElementsType() const
    {return doubleVectorClass(enumerationClass, doubleType);}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return inputType->getTemplateArgument(0);}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return -1;
    return v.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
  }

protected:
  friend class DoubleVectorLocalDimericCompositionFunctionClass;
  
  DoubleVectorLocalDimericCompositionFunction() {}
};

class ProbabilityLocalDimericCompositionFunction : public LocalDimericCompositionFunction
{
public:
  ProbabilityLocalDimericCompositionFunction(size_t windowSize)
  : LocalDimericCompositionFunction(windowSize) {}

protected:
  virtual TypePtr getInputElementsType() const
    {return probabilityType;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return falseOrTrueEnumeration;}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return -1;
    return v.getDouble() >= 0.5 ? 1 : 0;
  }
  
protected:
  friend class ProbabilityLocalDimericCompositionFunctionClass;
  
  ProbabilityLocalDimericCompositionFunction() {}
};

class LocalDimericCompositionProxyFunction : public ProxyFunction
{
public:
  LocalDimericCompositionProxyFunction(size_t windowSize)
    : windowSize(windowSize) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual String getOutputPostFix() const
    {return T("DimerProxy");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(anyType);}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputType = inputVariables[0]->getType();

    if (inputType == probabilityType)
      return new ProbabilityLocalDimericCompositionFunction(windowSize);
    else if (inputType == aminoAcidTypeEnumeration)
      return new AminoAcidLocalDimericCompositionFunction(windowSize);
    else if (inputType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)))
      return new DoubleVectorLocalDimericCompositionFunction(windowSize);
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }

protected:
  friend class LocalDimericCompositionProxyFunctionClass;
  
  size_t windowSize;

  LocalDimericCompositionProxyFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DIMERIC_COMPOSITION_FUNCTION_H_
