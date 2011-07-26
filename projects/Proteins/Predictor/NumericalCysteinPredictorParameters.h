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
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include "ConnectivityPatternClassifier.h"

namespace lbcpp
{

extern ClassPtr numericalCysteinFeaturesParametersClass;

class NumericalCysteinFeaturesParameters;
typedef ReferenceCountedObjectPtr<NumericalCysteinFeaturesParameters> NumericalCysteinFeaturesParametersPtr;

class NumericalCysteinFeaturesParameters : public Object
{
public:
  bool useAminoAcid;
  bool usePSSM;
  
  bool useGlobalHistogram;
  
  bool useNumCysteins;
  bool useDiscretizeNumCysteins;
  bool useCysteinParity;

  bool useProteinLength;
  bool useDiscretizeProteinLength;

  size_t residueWindowSize;
  size_t localHistogramSize;
  size_t separationProfilSize;

  bool useSymmetricFeature;
  bool useIntervalHistogram;
  bool useAADistance;
  size_t pairWindowSize;
  size_t normalizedWindowSize;

  bool useExtendedD1Feature;
  size_t d1WindowSize;
  bool useExtendedD2Feature;
  
  bool useCysteinPositionDifference;
  bool useCysteinIndexDifference;
  
  bool useConnectivityPattern;
  bool useCartesianConnectivity;
  bool useConnectivityStats;
  
  bool useCysteinDistance; // like CysteinIndexDifference but not normalized (and hardDiscretize)
  bool useCommonNeighbors;
  bool useJaccardsCoef;
  bool useAdomicAdar;
  bool useAttachement;

  NumericalCysteinFeaturesParameters()
  : useAminoAcid(true), usePSSM(true)
  , useGlobalHistogram(true)

  , useNumCysteins(false), useDiscretizeNumCysteins(true), useCysteinParity(true)
  , useProteinLength(false), useDiscretizeProteinLength(false)
  , residueWindowSize(20), localHistogramSize(60), separationProfilSize(15)
  , useSymmetricFeature(false), useIntervalHistogram(false), useAADistance(true)
  , pairWindowSize(21), normalizedWindowSize(21)
  , useExtendedD1Feature(false), d1WindowSize(0), useExtendedD2Feature(true)
  , useCysteinPositionDifference(false), useCysteinIndexDifference(true)
  , useConnectivityPattern(true), useCartesianConnectivity(false), useConnectivityStats(true)
  , useCysteinDistance(true)
  , useCommonNeighbors(true), useJaccardsCoef(true), useAdomicAdar(false), useAttachement(false)
  {}
  
  static std::vector<SamplerPtr> createSamplers()
  {
    ClassPtr thisType = numericalCysteinFeaturesParametersClass;
    const size_t n = thisType->getNumMemberVariables();
    std::vector<SamplerPtr> res(n);
    
    for (size_t i = 0; i < n; ++i)
    {
      if (thisType->getMemberVariableType(i)->inheritsFrom(booleanType))
        res[i] = bernoulliSampler(0.5, 0.0, 1.0);
      else if (thisType->getMemberVariableName(i) == T("residueWindowSize"))
        res[i] = discretizeSampler(gaussianSampler(20, 15), 0, 50);
      else if (thisType->getMemberVariableName(i) == T("localHistogramSize"))
        res[i] = discretizeSampler(gaussianSampler(60, 60), 0, 500);
      else if (thisType->getMemberVariableName(i) == T("separationProfilSize"))
        res[i] = discretizeSampler(gaussianSampler(15, 15), 0, 26);
      else if (thisType->getMemberVariableName(i) == T("pairWindowSize"))
        res[i] = discretizeSampler(gaussianSampler(20, 15), 0, 26);
      else if (thisType->getMemberVariableName(i) == T("normalizedWindowSize"))
        res[i] = discretizeSampler(gaussianSampler(15, 15), 0, 26);
      else if (thisType->getMemberVariableName(i) == T("d1WindowSize"))
        res[i] = discretizeSampler(gaussianSampler(3, 15), 0, 26);
      else
        jassertfalse;
    }

    return res;
  }
  
  static std::vector<StreamPtr> createStreams()
  {
    ClassPtr thisType = numericalCysteinFeaturesParametersClass;
    const size_t n = thisType->getNumMemberVariables();
    std::vector<StreamPtr> res(n);
    
    for (size_t i = 0; i < n; ++i)
    {
      if (thisType->getMemberVariableType(i)->inheritsFrom(booleanType))
        res[i] = booleanStream(true);
      else if (thisType->getMemberVariableName(i) == T("residueWindowSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(15);
        values.push_back(19);
        values.push_back(25);
        values.push_back(29);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (thisType->getMemberVariableName(i) == T("localHistogramSize"))
      {
        std::vector<int> values;
        values.push_back(20);
        values.push_back(50);
        values.push_back(100);
        values.push_back(150);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (thisType->getMemberVariableName(i) == T("separationProfilSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(3);
        values.push_back(9);
        values.push_back(11);
        values.push_back(15);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (thisType->getMemberVariableName(i) == T("pairWindowSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(15);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (thisType->getMemberVariableName(i) == T("normalizedWindowSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(15);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (thisType->getMemberVariableName(i) == T("d1WindowSize"))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(15);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else
        jassertfalse;
    }
    return res;
  }
  
  static NumericalCysteinFeaturesParametersPtr createInitialObject()
  {
    NumericalCysteinFeaturesParametersPtr res = new NumericalCysteinFeaturesParameters();

    const size_t n = res->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr currentType = res->getVariableType(i);
      if (currentType->inheritsFrom(booleanType))
        res->setVariable(i, Variable(false, booleanType));
      else if (currentType->inheritsFrom(integerType))
        res->setVariable(i, Variable(0, currentType));
      else if (currentType->inheritsFrom(doubleType))
        res->setVariable(i, Variable(0.f, currentType));
      else
        jassertfalse;
    }

    return res;
  }
  
  virtual String toString() const
    {return T("(") + defaultToStringImplementation(false) + T(")");}
  
  virtual bool loadFromString(ExecutionContext& context, const String& str)
  {
    if (str == T("Missing"))
      return true;
    if (str.length() < 2 || str[0] != '(' || str[str.length() - 1] != ')')
    {
      context.errorCallback(T("Invalid syntax: ") + str.quoted());
      return false;
    }
    return Object::loadFromString(context, str.substring(1, str.length() - 1));
  }

protected:
  friend class NumericalCysteinFeaturesParametersClass;
};

class NumericalCysteinPredictorParameters : public ProteinPredictorParameters
{
public:
  bool useAddBiasLearner;
  NumericalCysteinFeaturesParametersPtr fp;

  enum {maxProteinLengthOnSPX = 1733, maxProteinLengthOnSPXFromFile = 1395};

  bool useCartesianProduct;
  
  bool useOracleD0;
  bool useOracleD1;
  bool useOracleD2;

  NumericalCysteinPredictorParameters(NumericalCysteinFeaturesParametersPtr fp = new NumericalCysteinFeaturesParameters())
    : useAddBiasLearner(false)
    , fp(fp)
    , useCartesianProduct(false)
    // -----  Oracle  -----
    , useOracleD0(false), useOracleD1(false), useOracleD2(false)
    // ----- Features -----
    //, useCartesianProduct(false)
    // primary residue

    , learningParameters(new StochasticGDParameters(constantIterationFunction(1), /*maxIterationsWithoutImprovementStoppingCriterion(20)*/ StoppingCriterionPtr(), 500))
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
      if (fp->useProteinLength)
        builder.addFunction(new ProteinLengthNormalized(maxProteinLengthOnSPXFromFile), protein);
      if (fp->useDiscretizeProteinLength)
        builder.addFunction(new ProteinLengthFeatureGenerator(1000, 10), protein);
      // global composition
      if (fp->useGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("histogram"));
      // number of cysteins
      if (fp->useNumCysteins)
        builder.addFunction(new NumCysteinsFeatureGenerator(false), protein, T("numCysteins"));
      if (fp->useDiscretizeNumCysteins)
        builder.addFunction(new NumCysteinsFeatureGenerator(true), protein, T("numCysteins"));

      if (fp->useCysteinParity)
        builder.addFunction(new IsNumCysteinPair(), protein, T("isNumCysteinsPair"));

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }

  void primaryResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    /* Precompute */
    size_t aminoAcid = (size_t)-1;
    if (fp->useAminoAcid)
      aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), protein, position);
    
    size_t pssmRow = (size_t)-1;
    if (fp->usePSSM)
      pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("pssm"));

    /* feature generators */
    builder.startSelection();

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));
    
      if (fp->useAminoAcid)
        builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));

      if (fp->usePSSM)
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

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));
    
      if (fp->residueWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->residueWindowSize), primaryResidueFeatures, position, T("window"));

      if (fp->localHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->localHistogramSize), primaryResidueFeaturesAcc, position, T("localHisto[") + String((int)fp->localHistogramSize) + T("]"));

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

      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));

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

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));

      if (fp->separationProfilSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->separationProfilSize), cysteinSeparationProfil, cysteinIndex, T("cysSepProfil"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /*
  ************************ Residue Pair Perception ************************
  */
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  void residuePairFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType);
    size_t secondPosition = builder.addInput(positiveIntegerType);
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t primaryResidueFeaturesAcc = builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);

    size_t aaDist = (size_t)-1;
    if (fp->useAADistance)
      aaDist = builder.addFunction(new SubtractFunction(), secondPosition, firstPosition);

    builder.startSelection();

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));

      if (fp->useIntervalHistogram)
        builder.addFunction(accumulatorWindowMeanFunction(), primaryResidueFeaturesAcc, firstPosition, secondPosition, T("interval"));

      if (fp->useAADistance)
        builder.addFunction(hardDiscretizedNumberFeatureGenerator(0.f, (double)maxProteinLengthOnSPXFromFile, 20, false), aaDist, T("|AA2-AA1|"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  /*
  ************************ Cystein Residue Pair Perception ************************
  */
  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const {}

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

      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception, T("globalFeatures"));

      if (fp->useSymmetricFeature)
        builder.addFunction(new SumDoubleVectorFunction(), rf1, rf2, T("rf1+rf2"));
      else
      {
        builder.addInSelection(rf1);
        builder.addInSelection(rf2);
      }

      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, residuePairFeatures), firstPosition, secondPosition, proteinPerception, T("residuePairFeatures"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResidueFeatures), firstPosition, proteinPerception, T("cysteinResidueFeatures[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResidueFeatures), secondPosition, proteinPerception, T("cysteinResidueFeatures[second]"));
      builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, cysteinResiduePairFeatures), firstPosition, secondPosition, proteinPerception, T("cysteinResiduePairFeatures"));

    size_t features = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
    // Information from D0
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD0), proteinPerception, features);
    // Information from D1
    features = builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD1ToD2), proteinPerception, firstPosition, secondPosition, features);
    // Informaton from D2
    builder.addFunction(lbcppMemberCompositeFunction(NumericalCysteinPredictorParameters, includeD2ToD2), proteinPerception, firstPosition, secondPosition, features);
  }

  void cysteinResiduePairFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);
    
    size_t cysDist = (size_t)-1;
    if (fp->useCysteinDistance)
      cysDist = builder.addFunction(new SubtractFunction(), secondIndex, firstIndex);
  
    builder.startSelection();
    
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));
    
      // Lin09 features
      if (fp->useCysteinPositionDifference)
        builder.addFunction(new NormalizedCysteinPositionDifference(), protein, firstPosition, secondPosition, T("NCPD"));
      if (fp->useCysteinIndexDifference)
        builder.addFunction(new NormalizedCysteinIndexDifference(), protein, firstIndex, secondIndex, T("NCID"));
    
      if (fp->useCysteinDistance)
        builder.addFunction(hardDiscretizedNumberFeatureGenerator(0.f, 26.f, 20, false), cysDist, T("|CYS2-CYS1|"));

    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
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
      
        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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

        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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

        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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
    size_t prob = builder.addFunction(new GetBondingStateProbabilities(fp->useExtendedD1Feature), cbs, cysteinIndex, T("p[D1]"));

    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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
      
        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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
    size_t probs = builder.addFunction(new GetPairBondingStateProbabilities(fp->useExtendedD1Feature, fp->d1WindowSize), cbs, firstIndex, secondIndex, T("P[D1]"));

    if (useCartesianProduct)
    {
      builder.startSelection();
      
        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
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
    
    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("disulfideBonds"));
    size_t dsbFeatures = builder.addFunction(mapContainerFunction(doubleFeatureGenerator()), dsb);
    size_t prob = builder.addFunction(new GetDisulfideBondProbability(), dsb, firstIndex, secondIndex, T("P[D2]"));
    
    size_t probGreedy = (size_t)-1;
    if (fp->useExtendedD2Feature)
    {
      size_t dsbGreedy = builder.addFunction(new GreedyDisulfidePatternBuilder(), dsb, T("greedy"));
      probGreedy = builder.addFunction(new GetDisulfideBondProbability(), dsbGreedy, firstIndex, secondIndex, T("P[Greedy(D2)]"));
    }
    
    size_t normalizedD2 = builder.addFunction(new NormalizeDisulfideBondFunction(), dsb);
    size_t normalizedD2Features = builder.addFunction(mapContainerFunction(doubleFeatureGenerator()), normalizedD2);

    size_t firstConnectivity = builder.addFunction(new SimpleConnectivityPatternFeatureGenerator(), dsb, firstIndex, T("Connectivity[first]"));
    size_t secondConnectivity = builder.addFunction(new SimpleConnectivityPatternFeatureGenerator(), dsb, secondIndex, T("Connectivity[second]"));

    
    if (useCartesianProduct)
    {
      builder.startSelection();

        builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 1.0), T("identity"));
        builder.addInSelection(prob);

        if (probGreedy != (size_t)-1)
          builder.addInSelection(probGreedy);

      size_t dsbFeatures = builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
      builder.addFunction(cartesianProductFeatureGenerator(true), features, dsbFeatures);
    }
    else
    {
      builder.startSelection();
      
        builder.addInSelection(features);
        builder.addInSelection(prob);

        if (probGreedy != (size_t)-1)
          builder.addInSelection(probGreedy);

        if (fp->pairWindowSize)
          builder.addFunction(matrixWindowFeatureGenerator(fp->pairWindowSize, fp->pairWindowSize)
                              , dsbFeatures, firstIndex, secondIndex, T("dsbWindow"));
        if (fp->normalizedWindowSize)
          builder.addFunction(matrixWindowFeatureGenerator(fp->normalizedWindowSize, fp->normalizedWindowSize)
                              , normalizedD2Features, firstIndex, secondIndex, T("normalizedD2Features"));

        if (fp->useConnectivityPattern)
        {
          builder.addInSelection(firstConnectivity);
          builder.addInSelection(secondConnectivity);
        }
      
        if (fp->useCartesianConnectivity)
          builder.addFunction(cartesianProductFeatureGenerator(true), firstConnectivity, secondConnectivity, T("Connectivity[first]xConnectivity[second]"));

        if (fp->useConnectivityStats)
        {
          builder.addFunction(new SymmetricMatrixRowStatisticsFeatureGenerator(), dsb, firstIndex, T("ConnectivityStats[first]"));
          builder.addFunction(new SymmetricMatrixRowStatisticsFeatureGenerator(), dsb, secondIndex, T("ConnectivityStats[second]"));
        }
      
        if (fp->useCommonNeighbors)
          builder.addFunction(new NumCommonNeighborsFeatureGenerator(), dsb, firstIndex, secondIndex, T("NumCommonNeighbors"));
      
        if (fp->useJaccardsCoef)
          builder.addFunction(new JaccardsCoefficientFeatureGenerator(), dsb, firstIndex, secondIndex, T("JaccardsCoef"));
      
        if (fp->useAdomicAdar)
          builder.addFunction(new AdamicAdarFeatureGenerator(), dsb, firstIndex, secondIndex, T("AdamicAdarsCoef"));
      
        if (fp->useAttachement)
          builder.addFunction(new PreferentialAttachementFeatureGenerator(), dsb, firstIndex, secondIndex, T("Attachement"));
      
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
    else if (useAddBiasLearner && target == dsbTarget)
      return new ConnectivityPatternClassifier(learningMachine(target), true);
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
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationAccuracyScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
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
