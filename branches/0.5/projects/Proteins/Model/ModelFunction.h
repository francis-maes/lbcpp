/*-----------------------------------------.---------------------------------.
| Filename: ModelFunction.h                | Model Function                  |
| Author  : Julien Becker                  |                                 |
| Started : 15/02/2013 21:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_MODEL_FUNCTION_H_
# define LBCPP_PROTEIN_MODEL_FUNCTION_H_

# include "../Data/Protein.h"

namespace lbcpp
{

class NumOfEachResidueFunction : public SimpleUnaryFunction
{
public:
  NumOfEachResidueFunction()
  : SimpleUnaryFunction(proteinClass,
                        denseDoubleVectorClass(standardAminoAcidTypeEnumeration, doubleType),
                        T("NumOfEachResidue")) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    if (!protein)
      return Variable::missingValue(positiveIntegerType);
    const size_t numStdAA = standardAminoAcidTypeEnumeration->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(standardAminoAcidTypeEnumeration, doubleType);
    const VectorPtr& ps = protein->getPrimaryStructure();
    const size_t n = ps->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const Variable v = ps->getElement(i);
      if (v.isMissingValue())
        continue;
      size_t aaIndex = ps->getElement(i).getInteger();
      if (aaIndex >= numStdAA)
        continue; // This is one of the non-standard AA
      res->getValueReference(aaIndex)++;
    }
    return res;
  }
};

class DimericAminoAcidCompositionFunction : public Function
{
public:
  virtual String getOutputPostFix() const
    {return T("DiComposition");}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return vectorClass(aminoAcidTypeEnumeration);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    return doubleVectorClass(cartesianProductEnumerationEnumeration(standardAminoAcidTypeEnumeration,
                                                                    standardAminoAcidTypeEnumeration), probabilityType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const VectorPtr& inputVector = input.getObjectAndCast<Vector>(context);
    if (!inputVector)
      return Variable::missingValue(getOutputType());
    
    const size_t numEnumerationElements = standardAminoAcidTypeEnumeration->getNumElements();
    const size_t numElements = inputVector->getNumElements();
    const double incrementValue = 1.f / (float)(numElements - 1.f);
    
    SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getOutputType());
    size_t previousIndex = (size_t)-1;
    for (size_t i = 0; i < numElements; ++i)
    {
      const Variable v = inputVector->getElement(i);
      if (v.isMissingValue())
      {
        previousIndex = (size_t)-1; 
        continue;
      }
      size_t currentIndex = v.getInteger();
      if (currentIndex >= numEnumerationElements)
      {
        previousIndex = (size_t)-1;
        continue; // Non standard amino acid
      }
      
      if (previousIndex != (size_t)-1)      
        res->incrementValue(previousIndex * numEnumerationElements + currentIndex, incrementValue);
      previousIndex = currentIndex;
    }
    return res;
  }
};

class DimericCompositionFunction : public Function
{
public:
  virtual String getOutputPostFix() const
  {return T("DiComposition");}
  
  virtual size_t getNumRequiredInputs() const
  {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {return vectorClass(doubleVectorClass(enumerationClass, probabilityType));}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    EnumerationPtr templateEnum = inputVariables[0]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    return doubleVectorClass(cartesianProductEnumerationEnumeration(templateEnum, templateEnum), probabilityType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const VectorPtr& inputVector = input.getObjectAndCast<Vector>(context);
    if (!inputVector)
      return Variable::missingValue(getOutputType());
    
    const size_t numEnumerationElements = inputVector->getElementsType()->getTemplateArgument(0).dynamicCast<Enumeration>()->getNumElements();
    const size_t numElements = inputVector->getNumElements();
    const double incrementValue = 1.f / (float)(numElements - 1.f);
    
    SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getOutputType());
    size_t previousIndex = (size_t)-1;
    for (size_t i = 0; i < numElements; ++i)
    {
      const Variable v = inputVector->getElement(i);
      if (v.isMissingValue())
      {
        previousIndex = (size_t)-1; 
        continue;
      }
      size_t currentIndex = v.getInteger();
      if (previousIndex != (size_t)-1)      
        res->incrementValue(previousIndex * numEnumerationElements + currentIndex, incrementValue);
      previousIndex = currentIndex;
    }
    return res;
  }
};

class LocalDimericAminoAcidCompositionFunction : public Function
{
public:
  LocalDimericAminoAcidCompositionFunction(size_t windowSize)
    : windowSize(windowSize) {}

  virtual String getOutputPostFix() const
    {return T("LocalDiComposition");}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {return index == 0 ? (TypePtr)vectorClass(aminoAcidTypeEnumeration) : positiveIntegerType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    return doubleVectorClass(cartesianProductEnumerationEnumeration(standardAminoAcidTypeEnumeration,
                                                                    standardAminoAcidTypeEnumeration), probabilityType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const VectorPtr& inputVector = inputs[0].getObjectAndCast<Vector>(context);
    if (!inputVector)
      return Variable::missingValue(getOutputType());
    const size_t position = inputs[1].getInteger();

    const size_t numEnumerationElements = standardAminoAcidTypeEnumeration->getNumElements();
    const size_t numElements = inputVector->getNumElements();
    const double incrementValue = 1.f / (float)(windowSize - 1.f);

    SparseDoubleVectorPtr res = new SparseDoubleVector((ClassPtr)getOutputType());
    size_t previousIndex = (size_t)-1;

    size_t firstIndex = (size_t)juce::jmax((int)(position - windowSize / 2), 0);
    size_t lastIndex = (size_t)juce::jmin((int)(position + windowSize / 2), (int)numElements - 1);
    for (size_t i = firstIndex; i <= lastIndex; ++i)
    {
      const Variable v = inputVector->getElement(i);
      if (v.isMissingValue())
      {
        previousIndex = (size_t)-1; 
        continue;
      }
      size_t currentIndex = v.getInteger();
      if (currentIndex >= numEnumerationElements)
      {
        previousIndex = (size_t)-1;
        continue; // Non standard amino acid
      }

      if (previousIndex != (size_t)-1)      
        res->incrementValue(previousIndex * numEnumerationElements + currentIndex, incrementValue);
      previousIndex = currentIndex;      
    }
    return res;
  }

protected:
  friend class LocalDimericAminoAcidCompositionFunctionClass;

  size_t windowSize;

  LocalDimericAminoAcidCompositionFunction() {}
};

class SeparationProfile : public Object
{
public:
  SeparationProfile(const EnumerationPtr& enumeration)
  {profiles.resize(enumeration->getNumElements());}
  
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

class CreateSeparationProfileFunction : public SimpleUnaryFunction
{
public:
  CreateSeparationProfileFunction()
  : SimpleUnaryFunction(vectorClass(aminoAcidTypeEnumeration),
                        separationProfileClass,
                        T("Profiles")) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    VectorPtr inputVector = input.getObjectAndCast<Vector>();
    if (!inputVector)
      return Variable::missingValue(getOutputType());
    
    const size_t numEnumerationElements = standardAminoAcidTypeEnumeration->getNumElements();
    SeparationProfilePtr res = new SeparationProfile(standardAminoAcidTypeEnumeration);
    std::vector< std::vector<size_t> >& profiles = res->getProfiles();
    for (size_t i = 0; i < numEnumerationElements; ++i)
      profiles[i].push_back(0);
    
    const size_t numElements = inputVector->getNumElements();
    for (size_t i = 0; i < numElements; ++i)
    {
      const Variable v = inputVector->getElement(i);
      if (v.isMissingValue())
        continue;
      size_t index = v.getInteger();
      if (index >= numEnumerationElements)
        continue; // Non standard amino acid
      
      profiles[index].push_back(i);
    }
    
    for (size_t i = 0; i < numEnumerationElements; ++i)
      profiles[i].push_back(numElements);
    
    return res;
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
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    enumeration = new DefaultEnumeration();
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
      if (position > profile[i])
        ++myIndex;
      else
        break;
    /*    
     std::cout << "Profile: ";
     for (size_t i = 0; i < profile.size(); ++i)
     std::cout << profile[i] << " ";
     std::cout << std::endl;
     std::cout << "Position: " << position << " myIndex: " << myIndex << std::endl;
     */
    DenseDoubleVectorPtr res = new DenseDoubleVector((ClassPtr)getOutputType());
    size_t resIndex = 0;
    for (int i = myIndex - (int)windowSize / 2 + 1; i <= myIndex; ++i)
    {
      int lowerIndex = juce::jmax(0, i);
      //      std::cout << "[-" << resIndex << "] " << lowerIndex << std::endl;
      res->setValue(resIndex++, position - profile[lowerIndex]);
    }
    
    ++myIndex;
    if (position == profile[myIndex])
      ++myIndex;
    
    for (int i = myIndex; i < myIndex + (int)windowSize / 2; ++i)
    {
      int upperIndex = juce::jmin(profile.size() - 1, i);
      //      std::cout << "[+" << resIndex << "] " << upperIndex << std::endl;
      res->setValue(resIndex++, profile[upperIndex] - position);
    }
    
    return res;
  }
  
protected:
  friend class GetSeparationProfileFunctionClass;
  
  size_t index;
  size_t windowSize;
  
  DefaultEnumerationPtr enumeration;
  
  GetSeparationProfileFunction() {}
};

  
}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_MODEL_FUNCTION_H_
