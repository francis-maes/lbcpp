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
# include "DimericCompositionFunction.h"
# include "SeparationProfileFunction.h"
# include "SegmentProfileFunction.h"
# include "../Predictor/LargeProteinPredictorParameters.h"

namespace lbcpp
{

class GetSimpleProteinMapElement : public GetProteinMapElement
{
public:
  GetSimpleProteinMapElement(const String& variableName)
    : GetProteinMapElement(variableName)
  {
    if (variableName == T("Sequence.primaryStructure"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::aaResidueFeatures);
    }
    else if (variableName == T("Sequence.positionSpecificScoringMatrix"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::pssmResidueFeatures);
    }
    else if (variableName == T("Sequence.secondaryStructure"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::ss3ResidueFeatures);
    }
    else if (variableName == T("Sequence.dsspSecondaryStructure"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::ss8ResidueFeatures);
    }
    else if (variableName == T("Sequence.solventAccessibilityAt20p"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::saResidueFeatures);
    }
    else if (variableName == T("Sequence.disorderRegions"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::drResidueFeatures);
    }
    else if (variableName == T("Sequence.structuralAlphabetSequence"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueFeatures);
      residueFunction = (CompositeFunctionBuilderFunction)(&GetSimpleProteinMapElement::stalResidueFeatures);
    }
    else if (variableName.startsWith(T("Accumulator")))
    {
      String target = variableName.substring(variableName.indexOfChar(T('.')) + 1);

      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, residueAccumulator);
      featuresFunction = new GetSimpleProteinMapElement(T("Sequence.") + target);
    }
    else if (variableName == T("NumCys"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, numCysteines);
    }
    else if (variableName == T("NumOfEachAA"))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, numOfEachAminoAcid);
    }
    else if (variableName.startsWith(T("Dimer.")))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, dimericComposition);
    }
    else if (variableName.startsWith(T("SepProfile.")))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, separationProfile);
    }
    else if (variableName.startsWith(T("SegProfile.")))
    {
      function = lbcppMemberCompositeUnlearnableFunction(GetSimpleProteinMapElement, segmentProfile);
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

  void dimericComposition(CompositeFunctionBuilder& builder) const
  {
    String target = variableName.substring(variableName.indexOfChar(T('.')) + 1);

    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);

    size_t sequence = builder.addFunction(getVariableFunction(target), protein);
    builder.addFunction(new DimericCompositionProxyFunction(), sequence, T("Dimer[") + target + T("]"));
  }

  void separationProfile(CompositeFunctionBuilder& builder) const
  {
    String target = variableName.substring(variableName.indexOfChar(T('.')) + 1);

    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);

    size_t sequence = builder.addFunction(getVariableFunction(target), protein);
    builder.addFunction(new CreateSeparationProfileProxyFunction(), sequence, T("SepProfile[") + target + ("]"));    
  }

  void segmentProfile(CompositeFunctionBuilder& builder) const
  {
    String target = variableName.substring(variableName.indexOfChar(T('.')) + 1);
    
    size_t proteinMap = builder.addInput(proteinMapClass);
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinMap);

    size_t sequence = builder.addFunction(getVariableFunction(target), protein);
    builder.addFunction(new SegmentContainerProxyFunction(), sequence, T("SegProfile[") + target + ("]"));    
  }
  
protected:
  friend class GetSimpleProteinMapElementClass;

  CompositeFunctionBuilderFunction residueFunction;
  FunctionPtr featuresFunction;

  GetSimpleProteinMapElement() {}
};

extern ClassPtr simpleProteinModelClass;

class SimpleProteinModel : public ProteinModel
{
public:
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  bool x3LowMemory;
  bool useAddBias;
  FunctionPtr biasFunction;

  /* Global Feature Parameters */
  bool useProteinLength;
  bool useNumCysteines;
  bool useNumOfEachResidue;

  bool aaGlobalHistogram;
  bool aaDimericProfile;

  bool pssmGlobalHistogram;
  bool pssmDimericProfile;
  
  bool ss3GlobalHistogram;
  bool ss3DimericProfile;
  
  bool drGlobalHistogram;
  bool drDimericProfile;
  
  bool saGlobalHistogram;
  bool saDimericProfile;

  /* Residue Feature Parameter */
  bool usePosition;
  bool useRelativePosition;

  size_t aaWindowSize;
  size_t aaLocalHistogramSize;
  size_t aaLocalDimericProfileSize;
  size_t aaSeparationProfileSize;
  size_t aaSegmentProfileSize;

  size_t pssmWindowSize;
  size_t pssmLocalHistogramSize;
  size_t pssmLocalDimericProfileSize;
  size_t pssmSeparationProfileSize;
  size_t pssmSegmentProfileSize;
  
  size_t ss3WindowSize;
  size_t ss3LocalHistogramSize;
  size_t ss3LocalDimericProfileSize;
  size_t ss3SeparationProfileSize;
  size_t ss3SegmentProfileSize;

  size_t drWindowSize;
  size_t drLocalHistogramSize;
  size_t drLocalDimericProfileSize;
  size_t drSeparationProfileSize;
  size_t drSegmentProfileSize;

  size_t saWindowSize;
  size_t saLocalHistogramSize;
  size_t saLocalDimericProfileSize;
  size_t saSeparationProfileSize;
  size_t saSegmentProfileSize;

  SimpleProteinModel(ProteinTarget target = noTarget)
    : ProteinModel(target),
    /* Machine Learning Parameters */
      x3Trees(10),
      x3Attributes(0),
      x3Splits(1),
      x3LowMemory(true),
      useAddBias(false),
      biasFunction(FunctionPtr()),
    /* Global Feature Parameters */
      useProteinLength(false),
      useNumCysteines(false),
      useNumOfEachResidue(false),

      aaGlobalHistogram(false),
      aaDimericProfile(false),
      
      pssmGlobalHistogram(false),
      pssmDimericProfile(false),

      ss3GlobalHistogram(false),
      ss3DimericProfile(false),

      drGlobalHistogram(false),
      drDimericProfile(false),

      saGlobalHistogram(false),
      saDimericProfile(false),

    /* Residue Feature Parameter */
      usePosition(false),
      useRelativePosition(false),

      aaWindowSize(0),
      aaLocalHistogramSize(0),
      aaLocalDimericProfileSize(0),
      aaSeparationProfileSize(0),
      aaSegmentProfileSize(0),

      pssmWindowSize(0),
      pssmLocalHistogramSize(0),
      pssmLocalDimericProfileSize(0),
      pssmSeparationProfileSize(0),
      pssmSegmentProfileSize(0),

      ss3WindowSize(0),
      ss3LocalHistogramSize(0),
      ss3LocalDimericProfileSize(0),
      ss3SeparationProfileSize(0),
      ss3SegmentProfileSize(0),

      drWindowSize(0),
      drLocalHistogramSize(0),
      drLocalDimericProfileSize(0),
      drSeparationProfileSize(0),
      drSegmentProfileSize(0),

      saWindowSize(0),
      saLocalHistogramSize(0),
      saLocalDimericProfileSize(0),
      saSeparationProfileSize(0),
      saSegmentProfileSize(0)
  {}
  
protected:
  friend class SimpleProteinModelClass;

  virtual FunctionPtr createMachineLearning(ExecutionContext& context) const
  {
    FunctionPtr res = extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits, x3LowMemory);
    if (biasFunction)
      res = new PreProcessCompositeFunction(res, composeFunction(biasFunction, signedScalarToProbabilityFunction()));
    return res;
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
                            builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.primaryStructure")), proteinMap, T("Acc[AA]"));
    size_t pssmAccumulator = !pssmGlobalHistogram ? (size_t)-1 :
                              builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.positionSpecificScoringMatrix")), proteinMap, T("Acc[PSSM]"));
    size_t ss3Accumulator = !ss3GlobalHistogram ? (size_t)-1 :
                             builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.secondaryStructure")), proteinMap, T("Acc[SS3]"));
    size_t drAccumulator = !drGlobalHistogram ? (size_t)-1 :
                            builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.disorderRegions")), proteinMap, T("Acc[DR]"));
    size_t saAccumulator = !saGlobalHistogram ? (size_t)-1 :
                            builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.solventAccessibilityAt20p")), proteinMap, T("Acc[SA]"));

    /* Output */
    builder.startSelection();
    {
      if (useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));
      if (useNumCysteines)
        builder.addFunction(integerFeatureGenerator(), numCysteines, T("#Cys"));
      if (useNumOfEachResidue)
        builder.addFunction(new GetSimpleProteinMapElement(T("NumOfEachAA")), proteinMap, T("NumOfEachAA"));

      // histograms
      if (aaGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      if (pssmGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));
      if (ss3GlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), ss3Accumulator, T("h(SS3)"));
      if (drGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), drAccumulator, T("h(DR)"));
      if (saGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), saAccumulator, T("h(SA)"));

      // dimeric profiles
      if (aaDimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer.primaryStructure")), proteinMap, T("Dimer[AA]"));
      if (pssmDimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer.positionSpecificScoringMatrix")), proteinMap, T("Dimer[PSSM]"));
      if (ss3DimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer.secondaryStructure")), proteinMap, T("Dimer[SS3]"));
      if (drDimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer.disorderRegions")), proteinMap, T("Dimer[DR]"));
      if (saDimericProfile)
        builder.addFunction(new GetSimpleProteinMapElement(T("Dimer.solventAccessibilityAt20p")), proteinMap, T("Dimer[SA]"));

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

    size_t length = !useRelativePosition && !usePosition ? (size_t)-1 :
                     builder.addFunction(getVariableFunction(T("length")), proteinMap);

    size_t aa = !aaWindowSize ? (size_t)-1 :
                 builder.addFunction(new GetSimpleProteinMapElement(T("Sequence.primaryStructure")), proteinMap, T("Seq[AA]"));
    size_t aaAcc = !aaLocalHistogramSize ? (size_t)-1 :
                    builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.primaryStructure")), proteinMap, T("Acc[AA]"));
    size_t aaSepPro = !aaSeparationProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile.primaryStructure")), proteinMap);
    size_t aaSeq = !aaLocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("primaryStructure")), protein);
    size_t aaSegPro = !aaSegmentProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SegProfile.primaryStructure")), proteinMap);

    size_t pssm = !pssmWindowSize ? (size_t)-1 :
                   builder.addFunction(new GetSimpleProteinMapElement(T("Sequence.positionSpecificScoringMatrix")), proteinMap, T("Seq[normPSSM]"));
    size_t pssmAcc = !pssmLocalHistogramSize ? (size_t)-1 :
                      builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.positionSpecificScoringMatrix")), proteinMap, T("Acc[PSSM]"));
    size_t pssmSepPro = !pssmSeparationProfileSize ? (size_t)-1 :
                         builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile.positionSpecificScoringMatrix")), proteinMap);
    size_t pssmSeq = !pssmLocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("positionSpecificScoringMatrix")), protein);
    size_t pssmSegPro = !pssmSegmentProfileSize ? (size_t)-1 :
                         builder.addFunction(new GetSimpleProteinMapElement(T("SegProfile.positionSpecificScoringMatrix")), proteinMap);

    size_t ss3 = !ss3WindowSize ? (size_t)-1 :
                  builder.addFunction(new GetSimpleProteinMapElement(T("Sequence.secondaryStructure")), proteinMap, T("Seq[SS3]"));
    size_t ss3Acc = !ss3LocalHistogramSize ? (size_t)-1 :
                     builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.secondaryStructure")), proteinMap, T("Acc[SS3]"));
    size_t ss3SepPro = !ss3SeparationProfileSize ? (size_t)-1 :
                        builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile.secondaryStructure")), proteinMap);
    size_t ss3Seq = !ss3LocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("secondaryStructure")), protein);
    size_t ss3SegPro = !ss3SegmentProfileSize ? (size_t)-1 :
                        builder.addFunction(new GetSimpleProteinMapElement(T("SegProfile.secondaryStructure")), proteinMap);

    size_t dr = !drWindowSize ? (size_t)-1 :
                 builder.addFunction(new GetSimpleProteinMapElement(T("Sequence.disorderRegions")), proteinMap, T("Seq[DR]"));
    size_t drAcc = !drLocalHistogramSize ? (size_t)-1 :
                    builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.disorderRegions")), proteinMap, T("Acc[DR]"));
    size_t drSepPro = !drSeparationProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile.disorderRegions")), proteinMap);
    size_t drSeq = !drLocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("disorderRegions")), protein);
    size_t drSegPro = !drSegmentProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SegProfile.disorderRegions")), proteinMap);

    size_t sa = !saWindowSize ? (size_t)-1 :
                 builder.addFunction(new GetSimpleProteinMapElement(T("Sequence.solventAccessibilityAt20p")), proteinMap, T("Seq[SA]"));
    size_t saAcc = !saLocalHistogramSize ? (size_t)-1 :
                    builder.addFunction(new GetSimpleProteinMapElement(T("Accumulator.solventAccessibilityAt20p")), proteinMap, T("Acc[SA]"));
    size_t saSepPro = !saSeparationProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SepProfile.solventAccessibilityAt20p")), proteinMap);
    size_t saSeq = !saLocalDimericProfileSize ? (size_t)-1 :
                    builder.addFunction(getVariableFunction(T("solventAccessibilityAt20p")), protein);
    size_t saSegPro = !saSegmentProfileSize ? (size_t)-1 :
                       builder.addFunction(new GetSimpleProteinMapElement(T("SegProfile.solventAccessibilityAt20p")), proteinMap);

    /* Output */
    builder.startSelection();
    {
      if (usePosition)
      {
        builder.addFunction(integerFeatureGenerator(), position, T("position"));
        builder.addFunction(new SubtractFunction(), length, position, T("length-position"));        
      }
      if (useRelativePosition)
        builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("pos/len"));

      // window sizes
      if (aaWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(aaWindowSize), aa, position, T("w(AA,") + String((int)aaWindowSize) + (")"));
      if (pssmWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(pssmWindowSize), pssm, position, T("w(PSSM,") + String((int)pssmWindowSize) + (")"));
      if (ss3WindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(ss3WindowSize), ss3, position, T("w(SS3,") + String((int)ss3WindowSize) + T(")"));
      if (drWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(drWindowSize), dr, position, T("w(DR,") + String((int)drWindowSize) + T(")"));
      if (saWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(saWindowSize), sa, position, T("w(SA,") + String((int)saWindowSize) + T(")"));

      // local histograms
      if (aaLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(aaLocalHistogramSize), aaAcc, position, T("h(AA,") + String((int)aaLocalHistogramSize) + T(")"));
      if (pssmLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(pssmLocalHistogramSize), pssmAcc, position, T("h(PSSM,") + String((int)pssmLocalHistogramSize) + T(")"));
      if (ss3LocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(ss3LocalHistogramSize), ss3Acc, position, T("h(SS3,") + String((int)ss3LocalHistogramSize) + T(")"));
      if (drLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(drLocalHistogramSize), drAcc, position, T("h(DR,") + String((int)drLocalHistogramSize) + T(")"));
      if (saLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(saLocalHistogramSize), saAcc, position, T("h(SA,") + String((int)saLocalHistogramSize) + T(")"));

      // separation profiles
      if (aaSeparationProfileSize)
        for (size_t i = 0; i < standardAminoAcidTypeEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, aaSeparationProfileSize), aaSepPro, position, T("SepProfile[AA]"));
      if (pssmSeparationProfileSize)
        for (size_t i = 0; i < positionSpecificScoringMatrixEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, pssmSeparationProfileSize), pssmSepPro, position, T("SepProfile[PSSM]"));
      if (ss3SeparationProfileSize)
        for (size_t i = 0; i < secondaryStructureElementEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, ss3SeparationProfileSize), ss3SepPro, position, T("SepProfile[SS3]"));
      if (drSeparationProfileSize)
        for (size_t i = 0; i < falseOrTrueEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, drSeparationProfileSize), drSepPro, position, T("SepProfile[DR]"));
      if (saSeparationProfileSize)
        for (size_t i = 0; i < falseOrTrueEnumeration->getNumElements(); ++i)
          builder.addFunction(new GetSeparationProfileFunction(i, saSeparationProfileSize), saSepPro, position, T("SepProfile[SA]"));
      
      // dimeric local profiles
      if (aaLocalDimericProfileSize)
        builder.addFunction(new LocalDimericCompositionProxyFunction(aaLocalDimericProfileSize), aaSeq, position, T("Di(AA,") + String((int)aaLocalHistogramSize) + T(")"));
      if (pssmLocalDimericProfileSize)
        builder.addFunction(new LocalDimericCompositionProxyFunction(pssmLocalDimericProfileSize), pssmSeq, position, T("Di(PSSM,") + String((int)pssmLocalHistogramSize) + T(")"));
      if (ss3LocalDimericProfileSize)
        builder.addFunction(new LocalDimericCompositionProxyFunction(ss3LocalDimericProfileSize), ss3Seq, position, T("Di(SS3,") + String((int)ss3LocalHistogramSize) + T(")"));
      if (drLocalDimericProfileSize)
        builder.addFunction(new LocalDimericCompositionProxyFunction(drLocalDimericProfileSize), drSeq, position, T("Di(DR,") + String((int)drLocalHistogramSize) + T(")"));
      if (saLocalDimericProfileSize)
        builder.addFunction(new LocalDimericCompositionProxyFunction(saLocalDimericProfileSize), saSeq, position, T("Di(SA,") + String((int)saLocalHistogramSize) + T(")"));

      if (aaSegmentProfileSize)
        builder.addFunction(new LocalSegmentProfileFunction(aaSegmentProfileSize), aaSegPro, position, T("Segment(AA,") + String((int)aaSegmentProfileSize) + T(")"));
      if (pssmSegmentProfileSize)
        builder.addFunction(new LocalSegmentProfileFunction(pssmSegmentProfileSize), pssmSegPro, position, T("Segment(PSSM,") + String((int)pssmSegmentProfileSize) + T(")"));
      if (ss3SegmentProfileSize)
        builder.addFunction(new LocalSegmentProfileFunction(ss3SegmentProfileSize), ss3SegPro, position, T("Segment(SS3,") + String((int)ss3SegmentProfileSize) + T(")"));
      if (drSegmentProfileSize)
        builder.addFunction(new LocalSegmentProfileFunction(drSegmentProfileSize), drSegPro, position, T("Segment(DR,") + String((int)drSegmentProfileSize) + T(")"));
      if (saSegmentProfileSize)
        builder.addFunction(new LocalSegmentProfileFunction(saSegmentProfileSize), saSegPro, position, T("Segment(SA,") + String((int)saSegmentProfileSize) + T(")"));

      builder.addConstant(new DenseDoubleVector(emptyEnumeration, doubleType, 0, 1.0), T("empty")); // anti-crash
    }
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

public:
  static void createStreams(std::vector<StreamPtr>& result)
  {
    const size_t n = simpleProteinModelClass->getNumMemberVariables();
    result.clear();
    result.resize(n);
    const size_t firstFeatureIndex = simpleProteinModelClass->findMemberVariable(T("useProteinLength"));

    for (size_t i = firstFeatureIndex; i < n; ++i)
    {
      const TypePtr varType = simpleProteinModelClass->getMemberVariableType(i);
      const String varName = simpleProteinModelClass->getMemberVariableName(i);

      if (varType->inheritsFrom(booleanType))
        result[i] = booleanStream(true);
      else if (varName.endsWith(T("WindowSize")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);

        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("LocalHistogramSize")))
      {
        std::vector<int> values;
        for (int j = 10; j < 100; j += 10)
          values.push_back(j);
        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("SeparationProfileSize")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);

        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("SegmentProfileSize")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);
        
        result[i] = integerStream(positiveIntegerType, values);
      }
      if (varName.startsWith(T("LocalDimericProfileSize")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);

        result[i] = integerStream(positiveIntegerType, values);
      }
      else
        jassertfalse;
    }
  }

  static void createStreamsForDisorderedRegions(std::vector<StreamPtr>& result)
  {
    const size_t n = simpleProteinModelClass->getNumMemberVariables();
    result.clear();
    result.resize(n);
    const size_t firstFeatureIndex = simpleProteinModelClass->findMemberVariable(T("useProteinLength"));
    
    for (size_t i = firstFeatureIndex; i < n; ++i)
    {
      const TypePtr varType = simpleProteinModelClass->getMemberVariableType(i);
      const String varName = simpleProteinModelClass->getMemberVariableName(i);
      
      if (varName == T("drGlobalHistogram"))
        result[i] = booleanStream(true);
      else if (varName == T("drDimericProfile"))
        result[i] = booleanStream(true);        
      else if (varName == T("drWindowSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);
        values.push_back(25);
        
        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("drLocalHistogramSize"))
      {
        std::vector<int> values;
        for (int j = 10; j < 100; j += 10)
          values.push_back(j);
        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("drSeparationProfileSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);
        values.push_back(25);

        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("drSegmentProfileSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(11);
        values.push_back(15);
        values.push_back(21);
        values.push_back(25);

        result[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("drLocalDimericProfileSize"))
      {
        std::vector<int> values;
        for (int j = 10; j < 100; j += 20)
          values.push_back(j);
        
        result[i] = integerStream(positiveIntegerType, values);
      }
    }
  }

  static void createStreamsExceptFor(const String& prefix, std::vector<StreamPtr>& result)
  {
    SimpleProteinModel::createStreams(result);
    
    const size_t n = simpleProteinModelClass->getNumMemberVariables();
    const size_t firstFeatureIndex = simpleProteinModelClass->findMemberVariable(T("useProteinLength"));
    
    for (size_t i = firstFeatureIndex; i < n; ++i)
      if (simpleProteinModelClass->getMemberVariableName(i).startsWith(prefix))
        result[i] = StreamPtr();
  }
};

typedef ReferenceCountedObjectPtr<SimpleProteinModel> SimpleProteinModelPtr;

}; /* namespace lbcpp */

#endif // _PROTEINS_SIMPLE_MODEL_H_
