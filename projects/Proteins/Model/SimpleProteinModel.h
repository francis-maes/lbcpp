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
  
  void ss3ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass);
    
    builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position, T("S3"));
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

  /* Residue Feature Parameter */
  size_t aaWindowSize;
  size_t pssmWindowSize;

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
    /* Residue Feature Parameter */
      aaWindowSize(0),
      pssmWindowSize(0)
  {}

protected:
  friend class SimpleProteinModelClass;

  virtual FunctionPtr createMachineLearning(ExecutionContext& context) const
  {
    return extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits, false, x3LowMemory);
  }

  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinMap = builder.addInput(proteinMapClass);
    /* Data */
    size_t length = !useProteinLength ? (size_t)-1 :
                     builder.addFunction(getVariableFunction(T("length")), proteinMap);
    size_t aaAccumulator = !aaGlobalHistogram ? (size_t)-1 :
                            builder.addFunction(new GetSimpleProteinMapElement(T("Acc[AA]")), proteinMap, T("Acc[AA]"));
    size_t pssmAccumulator = !pssmGlobalHistogram ? (size_t)-1 :
                              builder.addFunction(new GetSimpleProteinMapElement(T("Acc[PSSM]")), proteinMap, T("Acc[PSSM]"));
    /* Output */
    builder.startSelection();
    {
      if (useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));
      if (aaGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      if (pssmGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));

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
    size_t aa = !aaWindowSize ? (size_t)-1 :
                 builder.addFunction(new GetSimpleProteinMapElement(T("Seq[AA]")), proteinMap, T("Seq[AA]"));
    //size_t aaAccumulator = builder.addFunction(new GetSimpleProteinMapElement(T("Acc[AA]")), proteinMap, T("Acc[AA]"));

    size_t pssm = !pssmWindowSize ? (size_t)-1 :
                   builder.addFunction(new GetSimpleProteinMapElement(T("Seq[PSSM]")), proteinMap, T("Seq[PSSM]"));
    //size_t pssmAccumulator = builder.addFunction(new GetSimpleProteinMapElement(T("Acc[PSMM]")), proteinMap, T("Acc[PSSM]"));

    /* Output */
    builder.startSelection();
    {
      // window sizes
      if (aaWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(aaWindowSize), aa, position, T("w(AA,") + String((int)aaWindowSize) + (")"));
      if (pssmWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(pssmWindowSize), pssm, position, T("w(PSSM,") + String((int)pssmWindowSize) + (")"));

      builder.addConstant(new DenseDoubleVector(emptyEnumeration, doubleType, 0, 1.0), T("empty")); // anti-crash
    }
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
};

typedef ReferenceCountedObjectPtr<SimpleProteinModel> SimpleProteinModelPtr;

}; /* namespace lbcpp */

#endif // _PROTEINS_SIMPLE_MODEL_H_
