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

class EnsureProteinTargetIsComputedFunction : public SimpleUnaryFunction
{
public:
  EnsureProteinTargetIsComputedFunction(ProteinTarget target = noTarget)
  : SimpleUnaryFunction(proteinClass, nilType, T("ComputeTarget")),
    target(target) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    protein->getTargetOrComputeIfMissing(context, target);
    return Variable();
  }

protected:
  friend class EnsureProteinTargetIsComputedFunctionClass;

  ProteinTarget target;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_MODEL_FUNCTION_H_
