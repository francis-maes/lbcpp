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
  NumCysteinsFeatureGenerator(bool hardDiscretization = true, bool lazy = false) 
    : FeatureGenerator(lazy), hardDiscretization(hardDiscretization) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("NumCysteinsFeatures"));

    if (!hardDiscretization)
    {
      res->addElement(context, T("value"));
      return res;
    }

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

    if (!hardDiscretization)
    {
      callback.sense(0, (double)numCysteins);
      return;
    }

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
  
protected:
  bool hardDiscretization;
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
  enum {maxProteinLengthOnSPX = 1733, maxProteinLengthOnSPXFromFile = 1395};
  
  bool useOracleD0;
  bool useOracleD1;
  bool useOracleD2;
  
  bool useCartesianProduct;

  bool useAminoAcid;
  bool usePSSM;
  
  bool useGlobalFeature;
  bool useGlobalHistogram;
  bool useProteinLength;
  bool useDiscretizeProteinLength;
  bool useNumCysteins;
  bool useDiscretizeNumCysteins;
  bool useCysteinParity;
  
  size_t residueWindowSize;
  size_t localHistogramSize;
  size_t separationProfilSize;
  
  bool useSymmetricFeature;
  
  NumericalCysteinPredictorParameters()
    // -----  Oracle  -----
    : useOracleD0(false), useOracleD1(false), useOracleD2(false)
    // ----- Features -----
    , useCartesianProduct(true)
    // primary residue
    , useAminoAcid(true), usePSSM(true)
    // global
    , useGlobalFeature(true)
    , useGlobalHistogram(true)
    , useProteinLength(false), useDiscretizeProteinLength(false)
    , useNumCysteins(false), useDiscretizeNumCysteins(true), useCysteinParity(true)
  
    // residue
    , residueWindowSize(3)
    , localHistogramSize(100)
    , separationProfilSize(0)
  
    // pair
    , useSymmetricFeature(true)

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

    size_t features = builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));
    // Information from D0
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD0), proteinPerception, features, T("D0"));
    // Information from D1
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD1ToD0), proteinPerception, features, T("D1"));
    // Information from D2
    builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD2ToD0), proteinPerception, features, T("D2"));
  }
  
  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t protein = builder.addInput(proteinClass, T("protein"));
    size_t primaryFeaturesAcc = builder.addInput(containerClass(doubleVectorClass()));

    /* Data */
    builder.startSelection();
      // protein length
      if (useProteinLength)
        builder.addFunction(new ProteinLengthNormalized(maxProteinLengthOnSPXFromFile), protein);
      if (useDiscretizeProteinLength)
        builder.addFunction(new ProteinLengthFeatureGenerator(1000, 10), protein);
      // global composition
      if (useGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("histogram"));
      // number of cysteins
      if (useNumCysteins)
        builder.addFunction(new NumCysteinsFeatureGenerator(false), protein, T("numCysteins"));
      if (useDiscretizeNumCysteins)
        builder.addFunction(new NumCysteinsFeatureGenerator(true), protein, T("numCysteins"));

      if (useCysteinParity)
        builder.addFunction(new IsNumCysteinPair(), protein, T("isNumCysteinsPair"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
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

    /* feature generators */
    builder.startSelection();

      if (useAminoAcid)
        builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));

      if (usePSSM)
        builder.addInSelection(pssmRow);

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }

  /*
  ************************ Residue Perception ************************
  */
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t primaryResidueFeatures = builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
    size_t primaryResidueFeaturesAcc = builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);

    /* Output */
    builder.startSelection();

      builder.addConstant(0.f);
    
      if (residueWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(residueWindowSize), primaryResidueFeatures, position, T("window"));

      if (localHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(localHistogramSize), primaryResidueFeaturesAcc, position, T("localHisto[") + String((int)localHistogramSize) + T("]"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /*
  ************************ Cystein Residue Perception ************************
  */
  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateCysteinBondingStateVectorFunction(
            lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResiudeVectorFeatures))
                                        , T("cysteinBondingStateResidueFeature"));
  }

  void cysteinResiudeVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Output */
    builder.startSelection();

    size_t global = -1;  
    if (useGlobalFeature)
        global = builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));
// ________
#if 0
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
    
    //size_t aminoAcid = builder.addFunction(getVariableFunction(T("primaryStructure")), protein);
    size_t pssmRow = builder.addFunction(getVariableFunction(T("positionSpecificScoringMatrix")), protein, T("pssm"));

    //size_t aa = builder.addFunction(mapContainerFunction(enumerationFeatureGenerator()), aminoAcid, T("aa"));
    size_t pssm = builder.addFunction(centeredContainerWindowFeatureGenerator(1), pssmRow, position);

    //aa = builder.addFunction(centeredContainerWindowFeatureGenerator(3), aa, position);
    builder.addFunction(cartesianProductFeatureGenerator(true), global, pssm);
    //builder.addFunction(concatenateFeatureGenerator(true), global, aa);
    return;
#endif
// ________
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, residueFeatures), position, proteinPerception, T("residueFeatures"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResidueFeatures), position, proteinPerception, T("cysteinFeatures"));
    
    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    // Information from D0
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD0), proteinPerception, features);
    // Information from D1
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD1ToD1), proteinPerception, position, features);
    // Inforamtion from D2
    builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD2ToD1), proteinPerception, position, features);
  }

  void cysteinResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType);
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Cystein Separation Profil */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);

    size_t cysteinSeparationProfil = builder.addFunction(new CreateCysteinSeparationProfil(true), protein, position, T("cysSepProfil"));
    cysteinSeparationProfil = builder.addFunction(mapContainerFunction(doubleFeatureGenerator()), cysteinSeparationProfil);

    /* Output */
    builder.startSelection();

      builder.addConstant(0.f);

      if (separationProfilSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(separationProfilSize), cysteinSeparationProfil, cysteinIndex, T("cysSepProfil"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /*
  ************************ Residue Pair Perception ************************
  */
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  /*
  ************************ Cystein Residue Pair Perception ************************
  */
  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResiduePairVectorFeatures))
                                        , T("cysteinResiduePairFeatures"));
  }
  
  void cysteinResiduePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t rf1 = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, residueFeatures), firstPosition, proteinPerception, T("residueFeature[first]"));
    size_t rf2 = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, residueFeatures), secondPosition, proteinPerception, T("residueFeature[second]"));

    /* Output */
    builder.startSelection();

    if (useGlobalFeature)
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));

    if (useSymmetricFeature)
      builder.addFunction(new SumDoubleVectorFeatureGenerator(), rf1, rf2, T("rf1+rf2"));
    else
    {
      builder.addInSelection(rf1);
      builder.addInSelection(rf2);
    }

#if 0
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures), firstPosition, secondPosition, proteinPerception, T("residuePairFeatures"));

      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResidueFeatures), firstPosition, proteinPerception, T("cysteinResidueFeatures[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResidueFeatures), secondPosition, proteinPerception, T("cysteinResidueFeatures[second]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, cysteinResiduePairFeatures), firstPosition, secondPosition, proteinPerception, T("cysteinResiduePairFeatures"));    
#endif
    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    // Information from D0
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD0), proteinPerception, features);
    // Information from D1
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD1ToD2), proteinPerception, firstPosition, secondPosition, features);
    // Informaton from D2
    builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD2ToD2), proteinPerception, firstPosition, secondPosition, features);
  }

  void cysteinResiduePairFeatures(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  /*
   ************************ Multi Task Features ************************
   */
  void includeD0(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType), T("features"));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cbp = builder.addFunction(getVariableFunction(T("cysteinBondingProperty")), protein, T("cysteinBondingProperty"));
    
    size_t pAllIndex = builder.addConstant(Variable(0, positiveIntegerType));
    size_t pNoneIndex = builder.addConstant(Variable(1, positiveIntegerType));
    size_t pMixIndex = builder.addConstant(Variable(2, positiveIntegerType));

    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(1.f, T("identity"));
        builder.addFunction(new GetDoubleVectorValueFunction(), cbp, pAllIndex, T("p[All]"));
        builder.addFunction(new GetDoubleVectorValueFunction(), cbp, pMixIndex, T("p[Mix]"));
      
      size_t cbpFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, cbpFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addFunction(new GetDoubleVectorValueFunction(), cbp, pAllIndex, T("p[All]"));
        builder.addFunction(new GetDoubleVectorValueFunction(), cbp, pNoneIndex, T("p[None]"));
        builder.addFunction(new GetDoubleVectorValueFunction(), cbp, pMixIndex, T("p[Mix]"));
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD1ToD0(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cbs = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein, T("cysteinBondingProperty"));

    size_t cbsRatio = builder.addFunction(new CysteinBondingStateRatio(), cbs, T("ratio"));
    
    if (useCartesianProduct)
    {
      builder.startSelection();

        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(cbsRatio);
      
      size_t cbsFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, cbsFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(cbsRatio);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD2ToD0(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("cysteinBondingProperty"));

    size_t dsbGreedy = builder.addFunction(new GreedyDisulfidePatternBuilder(), dsb, T("greedy"));
    size_t dsbGreedyRatio = builder.addFunction(new GreedyDisulfideBondRatio(), dsbGreedy, T("ratio"));

    if (useCartesianProduct)
    {
      builder.startSelection();

        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(dsbGreedyRatio);

      size_t dsbFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, dsbFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(dsbGreedyRatio);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD1ToD1(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cbs = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein, T("cysteinBondingProperty"));
    
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);
    size_t prob = builder.addFunction(new GetDoubleVectorValueFunction(), cbs, cysteinIndex, T("p[D1]"));

    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(prob);
      
      size_t cbsFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, cbsFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(prob);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD2ToD1(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));

    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);

    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("cysteinBondingProperty"));
    size_t dsbGreedy = builder.addFunction(new GreedyDisulfidePatternBuilder(), dsb, T("greedy"));
    size_t dsbGreedySumRow = builder.addFunction(new GreedyDisulfideBondSumOfRow(), dsbGreedy, cysteinIndex, T("sumOfRow"));
    
    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(dsbGreedySumRow);
      
      size_t dsbFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, dsbFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(dsbGreedySumRow);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD1ToD2(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);

    size_t cbs = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein, T("cysteinBondingProperty"));
    size_t probs = builder.addFunction(new GetBondingStateProbabilities(), cbs, firstIndex, secondIndex, T("P[D1]"));

    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(probs);
      
      size_t cbsFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, cbsFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(probs);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }
  
  void includeD2ToD2(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType), T("proteinPerception"));
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t features = builder.addInput(doubleVectorClass(enumValueType, doubleType));
    
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);
    
    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("cysteinBondingProperty"));
    size_t prob = builder.addFunction(new GetDisulfideBondProbability(), dsb, firstIndex, secondIndex, T("P[D2]"));
    
    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(1.f, T("identity"));
        builder.addInSelection(prob);

      size_t dsbFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, dsbFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(prob);
      
      builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    }
  }

  // Learning Machine
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    if (target == cbpTarget && useOracleD0
        || target == cbsTarget && useOracleD1
        || target == dsbTarget && useOracleD2)
    {
      return new GetSupervisionFunction();
    }
    else
      return ProteinPredictorParameters::createTargetPredictor(target);
  }
  
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    switch (target)
    { // binaryClassificationAccuracyScore
    case cbpTarget:
      {
        FunctionPtr res = linearLearningMachine(new StochasticGDParameters(constantIterationFunction(1.f), StoppingCriterionPtr(), 1000));
        res->setEvaluator(defaultSupervisedEvaluator());
        return res;
      }
    case cbsTarget:
      {
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationAccuracyScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
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
