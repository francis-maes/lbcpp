/*-----------------------------------------.---------------------------------.
| Filename: SegmentProfileFunction.h       | Segment Profile Function        |
| Author  : Julien Becker                  |                                 |
| Started : 21/02/2013 12:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_SEGMENT_PROFILE_FUNCTION_H_
# define LBCPP_PROTEIN_SEGMENT_PROFILE_FUNCTION_H_

# include "../Data/Protein.h"
# include "../../../src/Data/Container/SegmentContainerFunction.h"

namespace lbcpp
{

class AminoAcidSegmentContainerFunction : public SegmentContainerFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return aminoAcidTypeEnumeration;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return standardAminoAcidTypeEnumeration;}

  virtual Variable getValue(const Variable& v) const
  {
    if (v.isMissingValue() || v.getInteger() >= 20)
      return Variable(-1, integerType);
    return v;
  }
};

class DoubleVectorSegmentContainerFunction : public SegmentContainerFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return doubleVectorClass(enumValueType, doubleType);}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return inputType->getTemplateArgument(0)->getTemplateArgument(0);}

  virtual Variable getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return Variable(-1, integerType);
    return Variable(v.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue(), integerType);
  }
};

class ProbabilitySegmentContainerFunction : public SegmentContainerFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return probabilityType;}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return falseOrTrueEnumeration;}

  virtual Variable getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return Variable(-1, integerType);
    return Variable(v.getDouble() >= 0.5 ? 1 : 0, integerType);
  }
};

class SegmentContainerProxyFunction : public ProxyFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual String getOutputPostFix() const
    {return T("SegmentProxy");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(anyType);}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputType = inputVariables[0]->getType()->getTemplateArgument(0);

    if (inputType == positiveIntegerEnumerationEnumeration
        && inputVariables[0]->getType()->getTemplateArgument(1) == probabilityType)
      return new ProbabilitySegmentContainerFunction();
    else if (inputType == aminoAcidTypeEnumeration)
      return new AminoAcidSegmentContainerFunction();
    else if (inputType->inheritsFrom(doubleVectorClass(enumValueType, doubleType)))
      return new DoubleVectorSegmentContainerFunction();
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }
};

class LocalSegmentProfileFunction : public Function
{
public:
  LocalSegmentProfileFunction(size_t windowSize)
    : windowSize(windowSize) {}

  virtual String getOutputPostFix() const
    {return T("Segmented");}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)segmentContainerClass(enumValueType) : positiveIntegerType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    ConcatenateEnumerationPtr enumeration = new ConcatenateEnumeration(T("LocalSegmentProfile"));
    const size_t halfSize = windowSize / 2;
    enumeration->reserveSubEnumerations(halfSize * 4 + 2);
    EnumerationPtr inputEnum = inputVariables[0]->getType()->getTemplateArgument(0);
    numEnumerationElements = inputEnum->getNumElements();
    for (int i = -halfSize; i <= (int)halfSize; ++i)
    {
      enumeration->addSubEnumeration(T("[") + String(i) + T("].type"), inputEnum);
      enumeration->addSubEnumeration(T("[") + String(i) + T("].length"), singletonEnumeration);
    }

    return denseDoubleVectorClass(enumeration, doubleType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    SegmentContainerPtr inputProfile = inputs[0].getObjectAndCast<SegmentContainer>();
    size_t position = inputs[1].getInteger();
    if (!inputProfile)
      return Variable::missingValue(getOutputType());
    jassert(position < inputProfile->getInputNumElements());

    const size_t halfSize = windowSize / 2;
    const size_t numSegments = inputProfile->getNumSegments();
    
    DenseDoubleVectorPtr res = new DenseDoubleVector((ClassPtr)getOutputType());
    const int segmentIndex = inputProfile->getSegmentIndex(position);
    size_t resIndex = 0;
    for (int i = segmentIndex - (int)halfSize; i <= segmentIndex + (int)halfSize; ++i)
    {
      if (i < 0)
      {
        resIndex += numEnumerationElements + 1;
        continue;
      }

      if (i >= (int)numSegments)
        break;

      Segment segment(inputProfile->getSegment(i), inputProfile);
      if (!segment.getValue().exists())
      {
        resIndex += numEnumerationElements + 1;
        continue;
      }
      res->setValue(resIndex + segment.getValue().getInteger(), 1.f);
      resIndex += numEnumerationElements;
      res->setValue(resIndex, segment.getLength());
      ++resIndex;
    }

    return res;
  }
  
protected:
  friend class LocalSegmentProfileFunctionClass;
  
  size_t windowSize;
  size_t numEnumerationElements;

  LocalSegmentProfileFunction() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_SEGMENT_PROFILE_FUNCTION_H_
