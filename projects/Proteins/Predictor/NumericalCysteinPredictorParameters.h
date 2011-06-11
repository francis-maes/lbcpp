/*-----------------------------------------.---------------------------------.
| Filename: NumericalCysteinPredictorPar..h| Numerical Cystein Predictor     |
| Author  : Julien Becker                  |  Parameters                     |
| Started : 11/06/2011 10:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_NUMERICAL_CYSTEIN_PARAMETERS_H_
# define LBCPP_PROTEINS_PREDICTOR_NUMERICAL_CYSTEIN_PARAMETERS_H_

# include "ProteinPredictor.h"
# include "ProteinPerception.h"
# include "ProteinPredictorParameters.h"
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

class ProteinLengthFeatureGenerator : public FeatureGenerator
{
public:
  ProteinLengthFeatureGenerator(size_t maxLength, size_t stepSize, bool lazy = false) 
    : FeatureGenerator(lazy), maxLength(maxLength), stepSize(stepSize)
    {jassert(stepSize && maxLength);}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("ProteinLengthFeatures"));

    const size_t numFeatures = maxLength / stepSize;
    for (size_t i = 0; i < numFeatures; ++i)
      res->addElement(context, T("[") + String((int)(i * stepSize)) + T(";") + String((int)((i + 1) * stepSize)) + T("["));
    res->addElement(context, T("[") + String((int)(numFeatures * stepSize)) + T(";+inf"));

    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);

    const size_t numFeatures = maxLength / stepSize;
    size_t index = protein->getLength() / stepSize;
    if (index > numFeatures)
      index = numFeatures;

    callback.sense(index, 1.0);
  }

protected:
  size_t maxLength;
  size_t stepSize;
};

class ProteinLengthNormalized : public SimpleUnaryFunction
{
public:
  ProteinLengthNormalized(size_t maxLength)
    : SimpleUnaryFunction(proteinClass, probabilityType, T("ProteinLengthNormalized")), maxLength(maxLength) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    
    const size_t length = protein->getLength();
    if (length > maxLength)
    {
      jassertfalse;
      return probability(1.f);
    }
    return probability(length / (double)maxLength);
  }

protected:
  size_t maxLength;
};

class NumCysteinsFeatureGenerator : public FeatureGenerator
{
public:
  NumCysteinsFeatureGenerator(bool lazy = false) 
    : FeatureGenerator(lazy) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("NumCysteinsFeatures"));

    res->addElement(context, T("1-"));
    res->addElement(context, T("2"));
    res->addElement(context, T("3"));
    res->addElement(context, T("4"));
    res->addElement(context, T("5"));
    res->addElement(context, T("[6;10["));
    res->addElement(context, T("[10;15["));
    res->addElement(context, T("[15;20["));
    res->addElement(context, T("[20;25["));
    res->addElement(context, T("25+"));
    
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);

    const size_t numCysteins = protein->getCysteinIndices().size();
    
    size_t index = 9; // 25+
    if (numCysteins <= 1)
      index = 0;
    else if (numCysteins == 2)
      index = 1;
    else if (numCysteins == 3)
      index = 2;
    else if (numCysteins == 4)
      index = 3;
    else if (numCysteins == 5)
      index = 4;
    else if (numCysteins < 10)
      index = 5;
    else if (numCysteins < 15)
      index = 6;
    else if (numCysteins < 20)
      index = 7;
    else if (numCysteins < 25)
      index = 8;

    callback.sense(index, 1.0);
  }
};

class IsNumCysteinPair : public SimpleUnaryFunction
{
public:
  IsNumCysteinPair()
    : SimpleUnaryFunction(proteinClass, probabilityType, T("IsNumCysteinPair")) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    
    const size_t numCysteins = protein->getCysteinIndices().size();
    return probability((numCysteins + 1) % 2);
  }

protected:
  size_t maxLength;
};

class NumericalCysteinPredictorParameters : public ProteinPredictorParameters
{
public:
  bool useGlobalHistogram;
  bool useAminoAcid;
  bool usePSSM;
  bool useProteinLength;
  bool useNumCysteins;
  bool useCysteinParity;
  
  NumericalCysteinPredictorParameters()
    : useGlobalHistogram(true), useAminoAcid(true), usePSSM(true)
    , useProteinLength(true), useNumCysteins(true), useCysteinParity(true)

    , learningParameters(new StochasticGDParameters(constantIterationFunction(0.1), /*maxIterationsWithoutImprovementStoppingCriterion(20)*/ StoppingCriterionPtr(), 1000))
    {}
  
  /*
  ************************ Protein Perception ************************
  */
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();

      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(new ProteinLengthFunction(), protein);
      builder.addFunction(new NumCysteinsFunction(), protein);
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, primaryResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, globalFeatures), protein, primaryFeaturesAcc, T("globalFeatures"));

    builder.finishSelectionWithFunction(new CreateProteinPerceptionFunction());
  }

  /*
  ************************ Property Perception ************************
  */
  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception);
    
    builder.finishSelection();
  }
  
  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t protein = builder.addInput(proteinClass, T("protein"));
    size_t primaryFeaturesAcc = builder.addInput(containerClass(doubleVectorClass()));

    /* Data */
    //size_t numCysteins = builder.addFunction(new NumCysteinsFunction(), protein, T("numCysteins"));
    //size_t cbp = builder.addFunction(getVariableFunction(T("cysteinBondingProperty")), protein, T("cysteinBondingProperty"));
    // Cystein Bonding State Ratio
    //size_t cbs = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein);
    //size_t cbsRatio = builder.addFunction(new CysteinBondingStateRatio(), cbs, T("cbsRatio"));

    builder.startSelection();
      // protein length
      if (useProteinLength)
      {
        //builder.addFunction(new ProteinLengthFeatureGenerator(1000, 10), protein);
        builder.addFunction(new ProteinLengthNormalized(1733), protein);
      }
      // global composition
      if (useGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("histogram"));
      // number of cysteins
      if (useNumCysteins)
        builder.addFunction(new NumCysteinsFeatureGenerator(), protein, T("numCysteins"));
    
      if (useCysteinParity)
        builder.addFunction(new IsNumCysteinPair(), protein, T("isNumCysteinsPair"));
      // cystein bonding property
      //if (featuresParameters->bondingPropertyDiscretization)
      //  builder.addFunction(enumerationDistributionFeatureGenerator(featuresParameters->bondingPropertyDiscretization, featuresParameters->bondingPropertyEntropyDiscretization), cbp);
      // cystein bonding state ratio
      //if (featuresParameters->cbsRatioDiscretization)
      //  builder.addFunction(defaultProbabilityFeatureGenerator(featuresParameters->cbsRatioDiscretization), cbsRatio);
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  void primaryResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    /* Precompute */
    size_t aminoAcid = (size_t)-1;
    if (useAminoAcid)
      aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), protein, position);
    
    size_t pssmRow = (size_t)-1;
    if (usePSSM)
      pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("pssm"));
    //size_t ss3 = builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position);
    //size_t ss8 = builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), protein, position);
    //size_t stal = builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), protein, position);
    //size_t sa20 = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), protein, position);
    //size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), protein, position);

    //size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);
    //size_t cbs = builder.addFunction(getElementInVariableFunction(T("cysteinBondingStates")), protein, cysteinIndex);

    /* feature generators */
    builder.startSelection();

      if (useAminoAcid)
        builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
      if (usePSSM)
        builder.addInSelection(pssmRow);

      //addEnumerationDistributionFeatureGenerator(builder, pssmRow, T("pssm"), featuresParameters->pssmDiscretization, featuresParameters->pssmEntropyDiscretization);
      //addEnumerationDistributionFeatureGenerator(builder, ss3, T("ss3"), featuresParameters->ss3Discretization, featuresParameters->ss3EntropyDiscretization);
      //addEnumerationDistributionFeatureGenerator(builder, ss8, T("ss8"), featuresParameters->ss8Discretization, featuresParameters->ss8EntropyDiscretization);
      //addEnumerationDistributionFeatureGenerator(builder, stal, T("stal"), featuresParameters->stalDiscretization, featuresParameters->stalEntropyDiscretization);
      //addBinaryDistributionFeatureGenerator(builder, sa20, T("sa20"), featuresParameters->sa20Discretization);
      //addBinaryDistributionFeatureGenerator(builder, dr, T("dr"), featuresParameters->drDiscretization);
      //addBinaryDistributionFeatureGenerator(builder, cbs, T("cbs"), featuresParameters->cbsDiscretization);

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }

  /*
  ************************ Residue Perception ************************
  */
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("length")), proteinPerception);
      builder.addInSelection(proteinPerception);
    
    builder.finishSelectionWithFunction(createVectorFunction(
            lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, residueVectorFeatures))
                                        , T("residueFeatureVectors"));
  }
  
  void residueVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("Position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));
      //builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), position, proteinPerception, T("residueFeatures"));
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t primaryResidueFeatures = builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
    size_t primaryResidueFeaturesAcc = builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);

    /* Output */
    builder.startSelection();

      if (featuresParameters->residueWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->residueWindowSize), primaryResidueFeatures, position, T("window"));

      if (featuresParameters->residueLocalMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueLocalMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueLocalMeanSize));

      if (featuresParameters->residueMediumMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueMediumMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueMediumMeanSize));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }
  
  /*
  ************************ Cystein Residue Perception ************************
  */
  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
  {
#if 0
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateCysteinBondingStateVectorFunction(
            lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResiudeVectorFeatures))
                                        , T("cysteinBondingStateResidueFeature"));
#endif
  }

  void cysteinResiudeVectorFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Output */
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), position, proteinPerception, T("residueFeatures"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResidueFeatures), position, proteinPerception, T("cysteinFeatures"));
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }

  void cysteinResidueFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType);
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Cystein Separation Profil */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);

    size_t cysteinSeparationProfil = builder.addFunction(new CreateCysteinSeparationProfil(), protein, position, T("cysSepProfil"));
    cysteinSeparationProfil = builder.addFunction(mapContainerFunction(defaultPositiveIntegerFeatureGenerator(featuresParameters->cbsSeparationProfilDiscretization, 3.0)), cysteinSeparationProfil);

    /* Structural features */
    size_t cbs = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein);
    size_t cbsDiscretized = builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator(featuresParameters->cbsDiscretization)), cbs);

    /* Cystein Bonding Property */
    size_t cbp = builder.addFunction(getVariableFunction(T("cysteinBondingProperty")), protein);
    
    /* Output */
    builder.startSelection();

      if (featuresParameters->cbsSeparationProfilSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cbsSeparationProfilSize), cysteinSeparationProfil, cysteinIndex, T("window"));
    
      size_t cbsDiscretizedWindow = (size_t)-1;
      if (featuresParameters->cbsDiscretization)
        cbsDiscretizedWindow = builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cbsWindowSize), cbsDiscretized, cysteinIndex, T("window"));

      if (featuresParameters->bondingPropertyDiscretization && featuresParameters->cbsDiscretization && featuresParameters->useCartesianCBPvsCBS)
        builder.addFunction(cartesianProductFeatureGenerator(true), cbp, cbsDiscretizedWindow, T("Product"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }
  
  /*
  ************************ Residue Pair Perception ************************
  */
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
#if 0
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("length")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(createSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures))
                                        , T("residuePairVectorFeatures"));
#endif
  }

  void residuePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    builder.startSelection();

      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));

      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), firstPosition, proteinPerception, T("residueFeature[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), secondPosition, proteinPerception, T("residueFeature[second]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures), firstPosition, secondPosition, proteinPerception, T("residuePairFeatures"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }

  void residuePairFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType);
    size_t secondPosition = builder.addInput(positiveIntegerType);
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t primaryResidueFeatures = builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
    size_t primaryResidueFeaturesAcc = builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);
    // AA Distance
    size_t aaDist = (size_t)-1;
    if (featuresParameters->aminoAcidDistanceDiscretization)
    {
      aaDist = builder.addFunction(new SubtractFunction(), secondPosition, firstPosition);
      aaDist = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, 3, featuresParameters->aminoAcidDistanceDiscretization, true), aaDist, T("aaDistance"));
    }
    // windows for cartesian product
    size_t cartesianFirstWindow = (size_t)-1;
    size_t cartesianSecondWindow = (size_t)-1;
    if (featuresParameters->cartesianProductPrimaryWindowSize)
    {
      cartesianFirstWindow = builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cartesianProductPrimaryWindowSize), primaryResidueFeatures, firstPosition);
      cartesianSecondWindow = builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cartesianProductPrimaryWindowSize), primaryResidueFeatures, secondPosition);
    }

    /* Output */
    builder.startSelection();

    if (featuresParameters->aminoAcidDistanceDiscretization)
      builder.addInSelection(aaDist);

    if (featuresParameters->useIntervalMean)
      builder.addFunction(accumulatorWindowMeanFunction(), primaryResidueFeaturesAcc, firstPosition, secondPosition, T("interval"));
    
    if (featuresParameters->cartesianProductPrimaryWindowSize)
      builder.addFunction(cartesianProductFeatureGenerator(true), cartesianFirstWindow, cartesianSecondWindow, T("cartesianProduct") + String((int)featuresParameters->cartesianProductPrimaryWindowSize));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }
  
  /*
  ************************ Cystein Residue Pair Perception ************************
  */
  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
#if 0
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResiduePairVectorFeatures))
                                        , T("cysteinResiduePairFeatures"));
#endif
  }
  
  void cysteinResiduePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    builder.startSelection();

      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));

      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), firstPosition, proteinPerception, T("residueFeature[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures), secondPosition, proteinPerception, T("residueFeature[second]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures), firstPosition, secondPosition, proteinPerception, T("residuePairFeatures"));

      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResidueFeatures), firstPosition, proteinPerception, T("cysteinResidueFeatures[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResidueFeatures), secondPosition, proteinPerception, T("cysteinResidueFeatures[second]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResiduePairFeatures), firstPosition, secondPosition, proteinPerception, T("cysteinResiduePairFeatures"));    

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }

  void cysteinResiduePairFeatures(CompositeFunctionBuilder& builder) const
  {
#if 0
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstCysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondCysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);

    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein);
    size_t normalizedDsb = builder.addFunction(new NormalizeDisulfideBondFunction(), dsb);
    
    size_t dsbFirstEntropy = builder.addFunction(new ComputeCysteinEntropy(), normalizedDsb, firstCysteinIndex, T("firstEntropy"));
    size_t dsbSecondEntropy = builder.addFunction(new ComputeCysteinEntropy(), normalizedDsb, secondCysteinIndex, T("secondEntropy"));

    /* Discretization */
    size_t discretizedDsb = (size_t)-1;
    size_t discretizedNormalizedDsb = (size_t)-1;
    if (featuresParameters->dsbDiscretization)
    {
      discretizedDsb = builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator(featuresParameters->dsbDiscretization)), dsb);
      discretizedNormalizedDsb = builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator(featuresParameters->dsbDiscretization)), normalizedDsb);
    }

    // Cystein bonding property
    size_t cbp = (size_t)-1;
    size_t firstWindowForCbp = (size_t)-1;
    size_t secondWindowForCbp = (size_t)-1;
    if (featuresParameters->bondingPropertyDiscretization && featuresParameters->dsbCartesianCbpSize)
    {
      cbp = builder.addFunction(getVariableFunction(T("cysteinBondingProperty")), protein, T("cysteinBondingProperty"));
      cbp = builder.addFunction(enumerationDistributionFeatureGenerator(featuresParameters->bondingPropertyDiscretization, featuresParameters->bondingPropertyEntropyDiscretization), cbp);
      firstWindowForCbp = builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbCartesianCbpSize, featuresParameters->dsbCartesianCbpSize), discretizedDsb, firstCysteinIndex, secondCysteinIndex, T("w1"));
      secondWindowForCbp = builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbCartesianCbpSize, featuresParameters->dsbCartesianCbpSize), discretizedNormalizedDsb, firstCysteinIndex, secondCysteinIndex, T("w2"));
    }

    // Cystein bonding state
    size_t firstCbs = (size_t)-1;
    size_t secondCbs = (size_t)-1;
    size_t firstWindowForCbs = (size_t)-1;
    size_t secondWindowForCbs = (size_t)-1;
    if (featuresParameters->cbsDiscretization && featuresParameters->dsbCartesianCbsSize)
    {
      firstCbs = builder.addFunction(getElementInVariableFunction(T("cysteinBondingStates")), protein, firstCysteinIndex, T("firstState"));
      secondCbs = builder.addFunction(getElementInVariableFunction(T("cysteinBondingStates")), protein, secondCysteinIndex, T("secondState"));
      firstCbs = builder.addFunction(defaultProbabilityFeatureGenerator(featuresParameters->cbsDiscretization), firstCbs);
      secondCbs = builder.addFunction(defaultProbabilityFeatureGenerator(featuresParameters->cbsDiscretization), secondCbs);
      firstWindowForCbs = builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbCartesianCbsSize, featuresParameters->dsbCartesianCbsSize), discretizedDsb, firstCysteinIndex, secondCysteinIndex, T("windowForCbs[first]"));
      secondWindowForCbs = builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbCartesianCbsSize, featuresParameters->dsbCartesianCbsSize), discretizedNormalizedDsb, firstCysteinIndex, secondCysteinIndex, T("windowForCbs[second]"));
    }
    
    /* Output */
    builder.startSelection();
      // window on dsb
      if (featuresParameters->dsbDiscretization && featuresParameters->dsbPairWindowRows && featuresParameters->dsbPairWindowColumns)
      {
        builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbPairWindowRows, featuresParameters->dsbPairWindowColumns)
                            , discretizedDsb, firstCysteinIndex, secondCysteinIndex, T("dsbWindowBoth"));
        builder.addFunction(matrixWindowFeatureGenerator(featuresParameters->dsbPairWindowRows, featuresParameters->dsbPairWindowColumns)
                            , discretizedNormalizedDsb, firstCysteinIndex, secondCysteinIndex, T("dsbWindowBoth"));
      }
      // entropies
      if (featuresParameters->dsbEntropyDiscretization)
      {
        builder.addFunction(defaultPositiveDoubleFeatureGenerator(featuresParameters->dsbEntropyDiscretization, -10, 1), dsbFirstEntropy);
        builder.addFunction(defaultPositiveDoubleFeatureGenerator(featuresParameters->dsbEntropyDiscretization, -10, 1), dsbSecondEntropy);
      }
      // Cystein bonding property
      if (featuresParameters->dsbDiscretization && featuresParameters->dsbCartesianCbpSize)
      {
        builder.addFunction(cartesianProductFeatureGenerator(true), firstWindowForCbp, cbp);
        builder.addFunction(cartesianProductFeatureGenerator(true), secondWindowForCbp, cbp);
      }
      // Cystein bonding state
      if (featuresParameters->cbsDiscretization && featuresParameters->dsbCartesianCbsSize)
      {
        builder.addFunction(cartesianProductFeatureGenerator(true), firstWindowForCbs, firstCbs);
        builder.addFunction(cartesianProductFeatureGenerator(true), firstWindowForCbs, secondCbs);

        builder.addFunction(cartesianProductFeatureGenerator(true), secondWindowForCbs, firstCbs);
        builder.addFunction(cartesianProductFeatureGenerator(true), secondWindowForCbs, secondCbs);
      }

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
#endif
  }

  // Learning Machine
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    switch (target)
    { // binaryClassificationAccuracyScore
    case cbpTarget:
      {
        FunctionPtr res = linearLearningMachine(learningParameters);
        res->setEvaluator(defaultSupervisedEvaluator());
        return res;
      }
    case cbsTarget:
      {
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationSensitivityAndSpecificityScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore));
        return res;
      }
    case dsbTarget:
      {
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationMCCScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore));
        return res;
      }

    default:
      {
        jassertfalse;
        return FunctionPtr();
      }
    };
  }

protected:
  friend class NumericalCysteinPredictorParametersClass;

  LearnerParametersPtr learningParameters;
};

typedef ReferenceCountedObjectPtr<NumericalCysteinPredictorParameters> NumericalCysteinPredictorParametersPtr;

extern ClassPtr numericalCysteinPredictorParametersClass;  

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PREDICTOR_NUMERICAL_CYSTEIN_PARAMETERS_H_
