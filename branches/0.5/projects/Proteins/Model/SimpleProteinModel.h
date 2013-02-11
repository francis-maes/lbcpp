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
# include "../Predictor/LargeProteinPredictorParameters.h"

namespace lbcpp
{

class SimpleProteinModel : public ProteinModel
{
public:
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  bool x3LowMemory;

  SimpleProteinModel()
    : ProteinModel(ss3Target),
      x3Trees(10),
      x3Attributes(0),
      x3Splits(1),
      x3LowMemory(true) {}

protected:
  virtual FunctionPtr createMachineLearning(ExecutionContext& context) const
  {
    return extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits, false, x3LowMemory);
  }

  virtual void buildPerception(CompositeFunctionBuilder& builder) const
  {
    size_t protein = builder.addInput(proteinClass);

    size_t preComputedPerception = builder.addFunction(lbcppMemberCompositeUnlearnableFunction(SimpleProteinModel, preComputePerception), protein, T("PreComputePerception"));
    
    builder.startSelection();
    {
      builder.addFunction(getVariableFunction(T("length")), preComputedPerception);
      builder.addInSelection(preComputedPerception);
    }
    builder.finishSelectionWithFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(SimpleProteinModel, residuePerception)), T("rfVector"));
  }

  void residuePerception(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t numCysteins = builder.addFunction(getVariableFunction(T("numCysteins")), proteinPerception, T("#Cys"));
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception, T("length"));
    size_t aaResidueFeatures = builder.addFunction(getVariableFunction(T("aaResidueFeatures")), proteinPerception, T("pssmRF"));
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccu"));
    size_t pssmResidueFeatures = builder.addFunction(getVariableFunction(T("pssmResidueFeatures")), proteinPerception, T("pssmRF"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccu"));

    /* Output */
    builder.startSelection();
    {
      /*** Global Features ***/
      builder.addFunction(integerFeatureGenerator(), length, T("length"));

      // global histograms
      builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));

      // number of cysteins
      builder.addFunction(integerFeatureGenerator(), numCysteins, T("#Cys"));
      builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));

      // bias (and anti-crash)
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));

      /*** Residue Features ***/
      builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("Pos/Len"));

      // window sizes
      builder.addFunction(centeredContainerWindowFeatureGenerator(15), aaResidueFeatures, position, T("w(AA,") + String(15) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(15), pssmResidueFeatures, position, T("w(PSSM,") + String(15) + (")"));
    }
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  void preComputePerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();
    {
      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(new ProteinLengthFunction(), protein);
      builder.addFunction(new NumCysteinsFunction(), protein);
      // AA
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, aaResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // PSSM
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, pssmResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SS3
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, ss3ResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SS8
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, ss8ResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SA
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, saResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // DR
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, drResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // STAL
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(LargeProteinPredictorParameters, stalResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
    }
    builder.finishSelectionWithFunction(new CreateLargeProteinPerception());
  }

  void aaResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), protein, position, T("aa"));
    builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
  }
  
  void pssmResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("pssm"));
  }
  
  void ss3ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position, T("ss3"));
  }
  
  void ss8ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), protein, position, T("ss8"));
  }
  
  void saResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t sa = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), protein, position, T("sa"));
    builder.addFunction(doubleFeatureGenerator(), sa, T("sa"));
  }
  
  void drResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), protein, position, T("dr"));
    builder.addFunction(doubleFeatureGenerator(), dr, T("dr"));
  }
  
  void stalResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), protein, position, T("stal"));
  }
};

}; /* namespace lbcpp */

#endif // _PROTEINS_SIMPLE_MODEL_H_
