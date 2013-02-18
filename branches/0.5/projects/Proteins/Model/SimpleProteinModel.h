/*-----------------------------------------.---------------------------------.
| Filename: SimpleProteinModel.h           |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 09/02/2012 14:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _PROTEINS_SIMPLE_MODEL_H_
# define _PROTEINS_SIMPLE_MODEL_H_

# include <lbcpp/Learning/DecisionTree.h>
# include "ProteinModel.h"
# include "ProteinMap.h"
# include "ModelFunction.h"
# include "../Predictor/LargeProteinPredictorParameters.h"

namespace lbcpp
{

class GetSimpleProteinMapElement : public GetProteinMapElement
{
public:
  GetSimpleProteinMapElement(const String& variableName)
    : GetProteinMapElement(variableName)
  {
    if (variableName == T("Seq[AA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::aaResidueFeatures);
    }
    else if (variableName == T("Seq[PSSM]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::pssmResidueFeatures);
    }
    else if (variableName == T("Seq[normPSSM]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::normPssmResidueFeatures);
    }
    else if (variableName == T("Seq[SS3]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::ss3ResidueFeatures);
    }
    else if (variableName == T("Seq[SS8]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::ss8ResidueFeatures);
    }
    else if (variableName == T("Seq[SA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::saResidueFeatures);
    }
    else if (variableName == T("Seq[DR]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::drResidueFeatures);
    }
    else if (variableName == T("Seq[StAl]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::stalResidueFeatures);
    }
    else if (variableName == T("Acc[AA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[AA]"));
    }
    else if (variableName == T("Acc[PSSM]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[PSSM]"));
    }
    else if (variableName == T("Acc[SS3]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[SS3]"));
    }
    else if (variableName == T("Acc[SS8]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[SS8]"));
    }
    else if (variableName == T("Acc[SA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[SA]"));
    }
    else if (variableName == T("Acc[DR]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[DR]"));
    }
    else if (variableName == T("Acc[StAl]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Seq[StAl]"));
    }
    else if (variableName == T("NumCys"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, numCysteines);
    }
    else if (variableName == T("NumOfEachAA"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, numOfEachAminoAcid);
    }
    else if (variableName == T("Dimer[AA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, dimericAminoAcidComposition);
    }
    else if (variableName == T("SepProfile[AA]"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, separationProfile);
    }
    else
      jassertfalse;
  }

  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);

    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinMap);
    builder.addFunction(createVectorFunction(new MethodBasedCompositeFunction(refCountedPointerFromThis(this), residueFunction)), length, protein);
  }
  
  void residueAccumulator(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);

    size_t features = builder.addFunction(featuresFunction, proteinMap);
    builder.addFunction(accumulateContainerFunction(), features);
  }

  void aaResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);

    size_t aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), protein, position, T("[]"));
    builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("AA"));
  }

  void pssmResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("PSSM"));
  }

  void normPssmResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t pssm = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("PSSM"));
    builder.addFunction(new NormalizePssmRowFunction(), pssm);
  }

  void ss3ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position, T("SS3"));
  }
  
  void ss8ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), protein, position, T("SS8"));
  }
  
  void saResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    size_t sa = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), protein, position, T("[]"));
    builder.addFunction(doubleFeatureGenerator(), sa, T("SA"));
  }
  
  void drResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), protein, position, T("[]"));
    builder.addFunction(doubleFeatureGenerator(), dr, T("DR"));
  }
  
  void stalResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), protein, position, T("StAl"));
  }

  void numCysteines(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);
    builder.addFunction(new NumCysteinsFunction(), protein, T("NumCys"));
  }
  
  void numOfEachAminoAcid(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);
    builder.addFunction(new NumOfEachResidueFunction(), protein, T("NumOfEachAA"));
  }

  void dimericAminoAcidComposition(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);
    size_t sequence = builder.addFunction(getVariableFunction(T("primaryStructure")), protein);
    builder.addFunction(new DimericAminoAcidCompositionFunction(), sequence, T("Dimer[AA]"));
  }

  void separationProfile(CompositeFunctionBuilder& builder) const
  {
    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);
    size_t sequence = builder.addFunction(getVariableFunction(T("primaryStructure")), protein);
    builder.addFunction(new CreateSeparationProfileFunction(), sequence, T("SepProfile[AA]"));    
  }

protected:
  friend class GetSimpleProteinMapElementClass;

  CompositeFunctionBuilderFunction residueFunction;
  FunctionPtr featuresFunction;

  GetSimpleProteinMapElement() {}
};

class SimpleProteinModel : public ProteinModel
{
public:
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  bool x3LowMemory;

  /* Global Feature Parameters */
  bool useProteinLength;
  bool aaGlobalHistogram;
  bool pssmGlobalHistogram;
  bool useNumCysteines;
  bool useNumOfEachResidue;
  bool aaDimericProfile;

  /* Residue Feature Parameter */
  bool usePosition;
  bool useRelativePosition;
  size_t aaWindowSize;
  size_t pssmWindowSize;
  size_t aaLocalHistogramSize;
  size_t pssmLocalHistogramSize;
  size_t aaSeparationProfileSize;
  size_t aaLocalDimericProfileSize;

  SimpleProteinModel(ProteinTarget target = noTarget)
    : ProteinModel(target),
    /* Machine Learning Parameters */
      x3Trees(10),
      x3Attributes(0),
      x3Splits(1),
      x3LowMemory(true),
    /* Global Feature Parameters */
      useProteinLength(false),
      aaGlobalHistogram(false),
      pssmGlobalHistogram(false),
      useNumCysteines(false),
      useNumOfEachResidue(false),
      aaDimericProfile(false),
    /* Residue Feature Parameter */
      usePosition(false),
      useRelativePosition(false),
      aaWindowSize(0),
      pssmWindowSize(0),
      aaLocalHistogramSize(0),
      pssmLocalHistogramSize(0),
      aaSeparationProfileSize(0),
      aaLocalDimericProfileSize(0)
  {}

protected:
  friend class SimpleProteinModelClass;

  virtual FunctionPtr createMachineLearning(ExecutionContext& context) const
  {
    return extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits, x3LowMemory);
  }

  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinMap = builder.addInput(proteinMapClass);
    /* Data */
    size_t length = !useProteinLength ? (size_t)-1 :
                     builder.addFunction(getVariableFunction(T("length")), proteinMap);
    size_t numCysteines = !useNumCysteines ? (size_t)-1 :
                           builder.addFunction(new GetSimpleProteinMapElement(T("NumCys")), proteinMap, T("NumCys"));
    size_t aaAccumulator = !aaGlobalHistogram ? (size_t)-1 :
                            builder.addFunction(new GetSimpleProteinMapElement(T("Acc[AA]")), proteinMap, T("Acc[AA]"));
    size_t pssmAccumulator = !pssmGlobalHistogram ? (size_t)-1 :
                              builder.addFunction(new GetSimpleProteinMapElement(T("Acc[PSSM]")), proteinMap, T("Acc[PSSM]"));

    /* Output */
    builder.startSelection();
    {
      if (useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));
      if (useNumCysteines)
        builder.addFunction(integerFeatureGenerator(), numCysteines, T("#Cys"));
      if (useNumOfEachResidue)
        builder.addFunction(new GetSimpleProteinMapElement(T("NumOfEachAA")), proteinMap, T("NumOfEachAA"));

      if (aaGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      if (pssmGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));

      if (aaDimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer[AA]")), proteinMap, T("Dimer[AA]"));

      builder.addConstant(new DenseDoubleVector(emptyEnumeration, doubleType, 0, 1.0), T("empty")); // anti-crash
    }
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType);
    size_t proteinMap = builder.addInput(proteinMapClass);
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);

    size_t length = !useRelativePosition ? (size_t)-1 :
                     builder.addFunction(getVariableFunction(T("length")), proteinMap);

    size_t aa = !aaWindowSize ? (size_t)-1 :
                 builder.addFunction(new GetSimpleProteinMapElement(T("Seq[AA]")), proteinMap, T("Seq[AA]"));
    size_t aaAcc = !aaLocalHistogramSize ? (size_t)-1 :
                    builder.addFunction(new GetSimpleProteinMapElement(T("Acc[AA]")), proteinMap, T("Acc[AA]"));

    size_t pssm = !pssmWindowSize ? (size_t)-1 :
                   builder.addFunction(new GetSimpleProteinMapElement(T("Seq[normPSSM]")), proteinMap, T("Seq[normPSSM]"));
    size_t pssmAcc = !pssmLocalHistogramSize ? (size_t)-1 :
                      builder.addFunction(new GetSimpleProteinMapElement(T("Acc[PSMM]")), proteinMap, T("Acc[PSSM]"));

    size_t aaSepPro = !aaSeparationProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile[AA]")), proteinMap);
    size_t aaSeq = !aaLocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("primaryStructure")), protein);
    /* Output */
    builder.startSelection();
    {
      if (usePosition)
        builder.addFunction(integerFeatureGenerator(), position, T("position"));
      if (useRelativePosition)
        builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("pos/len"));

      // window sizes
      if (aaWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(aaWindowSize), aa, position, T("w(AA,") + String((int)aaWindowSize) + (")"));
      if (pssmWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(pssmWindowSize), pssm, position, T("w(PSSM,") + String((int)pssmWindowSize) + (")"));

      // local histograms
      if (aaLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(aaLocalHistogramSize), aaAcc, position, T("h(AA,") + String((int)aaLocalHistogramSize) + T(")"));
      if (pssmLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(pssmLocalHistogramSize), pssmAcc, position, T("h(PSSM,") + String((int)pssmLocalHistogramSize) + T(")"));

      if (aaSeparationProfileSize)
        for (size_t i = 0; i < standardAminoAcidTypeEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, aaSeparationProfileSize), aaSepPro, position, T("SepProfile[AA]"));

      if (aaLocalDimericProfileSize)
        builder.addFunction(new LocalDimericAminoAcidCompositionFunction(aaLocalDimericProfileSize), aaSeq, position, T("h(DiAA,") + String((int)aaLocalHistogramSize) + T(")"));

      builder.addConstant(new DenseDoubleVector(emptyEnumeration, doubleType, 0, 1.0), T("empty")); // anti-crash
    }
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
};

typedef ReferenceCountedObjectPtr<SimpleProteinModel> SimpleProteinModelPtr;

}; /* namespace lbcpp */

#endif // _PROTEINS_SIMPLE_MODEL_H_
