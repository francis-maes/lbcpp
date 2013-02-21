/*-----------------------------------------.---------------------------------.
| Filename: SeparationProfileFunction.h    | Separation Profile Function     |
| Author  : Julien Becker                  |                                 |
| Started : 21/02/2013 10:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEPARATION_PROFILE_FUNCTION_H_
# define LBCPP_SEPARATION_PROFILE_FUNCTION_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class SeparationProfile : public Object
{
public:
  SeparationProfile(const EnumerationPtr& enumeration)
    {profiles.resize(enumeration->getNumElements());}

  SeparationProfile(size_t numElements)
    {profiles.resize(numElements);}
  
  std::vector<size_t>& getProfile(size_t index)
    {jassert(index < profiles.size()); return profiles[index];}
  
  std::vector< std::vector<size_t> >& getProfiles()
    {return profiles;}
  
protected:
  friend class SeparationProfileClass;
  
  std::vector< std::vector<size_t> > profiles;
  
  SeparationProfile() {}
};

extern ClassPtr separationProfileClass;
typedef ReferenceCountedObjectPtr<SeparationProfile> SeparationProfilePtr;

class CreateSeparationProfileFunction : public Function
{
public:
  virtual String getOutputPostFix() const
    {return T("CreateProfile");}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(getInputElementsType());}
  
  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    numEnumerationElements = getOutputEnumeration(inputVariables[0]->getType())->getNumElements();
    return separationProfileClass;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    VectorPtr inputVector = input.getObjectAndCast<Vector>();
    if (!inputVector)
      return Variable::missingValue(getOutputType());

    SeparationProfilePtr res = new SeparationProfile(numEnumerationElements);
    std::vector< std::vector<size_t> >& profiles = res->getProfiles();
    for (size_t i = 0; i < numEnumerationElements; ++i)
      profiles[i].push_back(0);
    
    const size_t numElements = inputVector->getNumElements();
    for (size_t i = 0; i < numElements; ++i)
    {
      int value = getValue(inputVector->getElement(i));
      if (value >= 0)
        profiles[value].push_back(i);
    }

    for (size_t i = 0; i < numEnumerationElements; ++i)
      profiles[i].push_back(numElements);

    return res;
  }

protected:
  friend class CreateSeparationProfileFunctionClass;

  size_t numEnumerationElements;

  virtual TypePtr getInputElementsType() const = 0;
  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType) = 0;
  virtual int getValue(const Variable& v) const = 0;
};

class AminoAcidCreateSeparationProfileFunction : public CreateSeparationProfileFunction
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

class DoubleVectorCreateSeparationProfileFunction : public CreateSeparationProfileFunction
{
protected:
  virtual TypePtr getInputElementsType() const
    {return doubleVectorClass(enumValueType, doubleType);}

  virtual EnumerationPtr getOutputEnumeration(const TypePtr& inputType)
    {return inputType->getTemplateArgument(0)->getTemplateArgument(0);}

  virtual int getValue(const Variable& v) const
  {
    if (v.isMissingValue())
      return -1;
    return v.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
  }
};

class ProbabilityCreateSeparationProfileFunction : public CreateSeparationProfileFunction
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

class CreateSeparationProfileProxyFunction : public ProxyFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual String getOutputPostFix() const
    {return T("CreateProfileProxy");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(anyType);}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    TypePtr inputType = inputVariables[0]->getType()->getTemplateArgument(0);

    if (inputType == positiveIntegerEnumerationEnumeration
        && inputVariables[0]->getType()->getTemplateArgument(1) == probabilityType)
      return new ProbabilityCreateSeparationProfileFunction();
    else if (inputType == aminoAcidTypeEnumeration)
      return new AminoAcidCreateSeparationProfileFunction();
    else if (inputType->inheritsFrom(doubleVectorClass(enumValueType, doubleType)))
      return new DoubleVectorCreateSeparationProfileFunction();
    else
    {
      jassertfalse;
      return FunctionPtr();
    }
  }
};

class GetSeparationProfileFunction : public Function
{
public:
  GetSeparationProfileFunction(size_t elementEnumerationIndex, size_t windowSize)
    : index(elementEnumerationIndex), windowSize(windowSize) {}

  virtual String getOutputPostFix() const
    {return T("SepProfile<") + String((int)index) + T(">");}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)separationProfileClass : positiveIntegerType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr enumeration = new DefaultEnumeration();
    const size_t halfSize = windowSize / 2;
    for (size_t i = 0; i < halfSize; ++i)
      enumeration->addElement(context, T("[") + String(-(int)(halfSize + i)) + T("]"));
    for (size_t i = 0; i < halfSize; ++i)
      enumeration->addElement(context, T("[") + String((int)i + 1) + T("]"));
    return denseDoubleVectorClass(enumeration, doubleType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    SeparationProfilePtr inputProfile = inputs[0].getObjectAndCast<SeparationProfile>();
    size_t position = inputs[1].getInteger();
    if (!inputProfile)
      return Variable::missingValue(getOutputType());
    
    const std::vector<size_t>& profile = inputProfile->getProfile(index);
    int myIndex = 0;
    for (size_t i = 1; i < profile.size(); ++i)
    {
      if (position <= profile[i])
        break;
      ++myIndex;
    }

    DenseDoubleVectorPtr res = new DenseDoubleVector((ClassPtr)getOutputType());
    size_t resIndex = 0;
    for (int i = myIndex - (int)windowSize / 2 + 1; i <= myIndex; ++i)
    {
      int lowerIndex = juce::jmax(0, i);
      res->setValue(resIndex++, position - profile[lowerIndex]);
    }
    
    ++myIndex;
    if (position == profile[myIndex])
      ++myIndex;
    
    for (int i = myIndex; i < myIndex + (int)windowSize / 2; ++i)
    {
      int upperIndex = juce::jmin(profile.size() - 1, i);
      res->setValue(resIndex++, profile[upperIndex] - position);
    }

    return res;
  }
  
protected:
  friend class GetSeparationProfileFunctionClass;
  
  size_t index;
  size_t windowSize;

  GetSeparationProfileFunction() {}
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_SEPARATION_PROFILE_FUNCTION_H_
