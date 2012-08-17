/*-----------------------------------------.---------------------------------.
| Filename: LargeProteinPredictorParam...h | Large Protein Predictor         |
| Author  : Julien Becker                  |                     Parameters  |
| Started : 23/08/2011 16:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LARGE_H_
# define LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LARGE_H_

# include "ProteinPredictor.h"
# include "ProteinPerception.h"
# include "ProteinPredictorParameters.h"
# include "ConnectivityPatternClassifier.h"
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/NearestNeighbor.h>
# include <lbcpp/Learning/DecisionTree.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class PreProcessInputCompositeFunction : public CompositeFunction
{
public:
  PreProcessInputCompositeFunction(const FunctionPtr& f, const FunctionPtr& g)
    : f(f), g(g) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(anyType, T("input"));
    size_t supervision = builder.addInput(anyType, T("supervision"));

    size_t res = builder.addFunction(f, input);
    builder.addFunction(g, res, supervision);
  }

protected:
  friend class PreProcessInputCompositeFunctionClass;

  FunctionPtr f;
  FunctionPtr g;

  PreProcessInputCompositeFunction() {}
};

class PreProcessCompositeFunction : public CompositeFunction
{
public:
  PreProcessCompositeFunction(const FunctionPtr& f, const FunctionPtr& g)
    : f(f), g(g) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(anyType, T("input"));
    size_t supervision = builder.addInput(anyType, T("supervision"));

    size_t res = builder.addFunction(f, input, supervision);
    builder.addFunction(g, res, supervision);
  }

protected:
  friend class PreProcessCompositeFunctionClass;

  FunctionPtr f;
  FunctionPtr g;

  PreProcessCompositeFunction() {}
};

/*
** Large Protein Perception
*/
extern ClassPtr largeProteinPerceptionClass(TypePtr aaType = anyType, TypePtr pssmType = anyType,
                                            TypePtr ss3Type = anyType, TypePtr ss8Type = anyType,
                                            TypePtr saType = anyType, TypePtr drType = anyType,
                                            TypePtr stalType = anyType);

class LargeProteinPerception : public ProteinPrimaryPerception
{
public:
  VectorPtr aaResidueFeatures;
  ContainerPtr aaAccumulator;

  VectorPtr pssmResidueFeatures;
  ContainerPtr pssmAccumulator;

  VectorPtr ss3ResidueFeatures;
  ContainerPtr ss3Accumulator;

  VectorPtr ss8ResidueFeatures;
  ContainerPtr ss8Accumulator;

  VectorPtr saResidueFeatures;
  ContainerPtr saAccumulator;

  VectorPtr drResidueFeatures;
  ContainerPtr drAccumulator;

  VectorPtr stalResidueFeatures;
  ContainerPtr stalAccumulator;

  LargeProteinPerception(TypePtr aaType, TypePtr pssmType, TypePtr ss3Type, TypePtr ss8Type, TypePtr saType, TypePtr drType, TypePtr stalType)
    : ProteinPrimaryPerception(largeProteinPerceptionClass(aaType, pssmType, ss3Type, ss8Type, saType, drType, stalType)) {}
  LargeProteinPerception(TypePtr thisClass)
    : ProteinPrimaryPerception(thisClass) {}

protected:
  friend class LargeProteinPerceptionClass;

  LargeProteinPerception() {}
};

typedef ReferenceCountedObjectPtr<LargeProteinPerception> LargeProteinPerceptionPtr;

class CreateLargeProteinPerception : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return largeProteinPerceptionClass()->getNumMemberVariables();}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return largeProteinPerceptionClass()->getMemberVariableType(index);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    size_t index = largeProteinPerceptionClass()->getBaseType()->getNumMemberVariables();
    index += 1;
    TypePtr aaType = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr pssmType = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr ss3Type = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr ss8Type = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr saType = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr drType = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    index += 2;
    TypePtr stalType = inputVariables[index]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    return largeProteinPerceptionClass(aaType, pssmType, ss3Type, ss8Type, saType, drType, stalType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    LargeProteinPerceptionPtr res = new LargeProteinPerception(getOutputType());
    const size_t n = getNumInputs();
    for (size_t i = 0; i < n; ++i)
      res->setVariable(i, inputs[i]);
    return res;
  }
};

/*
** Large Protein Parameters
*/
class LargeProteinParameters;
typedef ReferenceCountedObjectPtr<LargeProteinParameters> LargeProteinParametersPtr;
extern ClassPtr largeProteinParametersClass;

class LargeProteinParameters : public Object
{
public:
  /* Global Features */
  bool useProteinLength;
  bool useNumCysteins;
  
  bool useAminoAcidGlobalHistogram;
  bool usePSSMGlobalHistogram;
  bool useSS3GlobalHistogram;
  bool useSS8GlobalHistogram;
  bool useSAGlobalHistogram;
  bool useDRGlobalHistogram;
  bool useSTALGlobalHistogram;

  /* Residue Features */
  bool useRelativePosition;
  bool useRelativeCysteinIndex;

  size_t aminoAcidWindowSize;
  size_t pssmWindowSize;
  size_t ss3WindowSize;
  size_t ss8WindowSize;
  size_t saWindowSize;
  size_t drWindowSize;
  size_t stalWindowSize;
  
  size_t aminoAcidLocalHistogramSize;
  size_t pssmLocalHistogramSize;
  size_t ss3LocalHistogramSize;
  size_t ss8LocalHistogramSize;
  size_t saLocalHistogramSize;
  size_t drLocalHistogramSize;
  size_t stalLocalHistogramSize;
  
  size_t separationProfilSize;
  
  /* Residue Pair Features */
  bool usePositionDifference;
  bool useIndexDifference;
  bool useCysteinDistance;
  bool useAADistance;

  bool useAminoAcidIntervalHistogram;
  bool usePSSMIntervalHistogram;
  bool useSS3IntervalHistogram;
  bool useSS8IntervalHistogram;
  bool useSAIntervalHistogram;
  bool useDRIntervalHistogram;
  bool useSTALIntervalHistogram;

  /* Cysteine Bonding State */
  size_t cbsAbsoluteSize;
  size_t cbsRelativeSize;
  size_t cbsMeanSize;
  size_t cbsStdDevSize;

  /* Disulfide Connectivity */
  size_t dsbProfilSize;
  size_t normalizedDsbProfilSize;
  bool useDsbProbability;
  bool useNormDsbProbability;

  LargeProteinParameters() :
    /* Global Features */
    useProteinLength(false), useNumCysteins(false),

    useAminoAcidGlobalHistogram(false), usePSSMGlobalHistogram(false),
    useSS3GlobalHistogram(false), useSS8GlobalHistogram(false), useSAGlobalHistogram(false),
    useDRGlobalHistogram(false), useSTALGlobalHistogram(false),

    /* Residue Features */
    useRelativePosition(false), useRelativeCysteinIndex(false),
    aminoAcidWindowSize(0), pssmWindowSize(0),
    ss3WindowSize(0), ss8WindowSize(0), saWindowSize(0),
    drWindowSize(0), stalWindowSize(0),

    aminoAcidLocalHistogramSize(0), pssmLocalHistogramSize(0),
    ss3LocalHistogramSize(0), ss8LocalHistogramSize(0), saLocalHistogramSize(0),
    drLocalHistogramSize(0), stalLocalHistogramSize(0),

    separationProfilSize(0),

    /* Residue Pair Features */
    usePositionDifference(false), useIndexDifference(false),
    useCysteinDistance(false), useAADistance(false),

    useAminoAcidIntervalHistogram(false), usePSSMIntervalHistogram(false),
    useSS3IntervalHistogram(false), useSS8IntervalHistogram(false),
    useSAIntervalHistogram(false), useDRIntervalHistogram(false),
    useSTALIntervalHistogram(false),
  
    /* Cysteine Bonding State */
    cbsAbsoluteSize(0), cbsRelativeSize(0), cbsMeanSize(0), cbsStdDevSize(0),

    /* Disulfide Connectivity */
    dsbProfilSize(0), normalizedDsbProfilSize(0),
    useDsbProbability(false), useNormDsbProbability(false)
  {}

  static std::vector<StreamPtr> createStreams()
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<StreamPtr> res(n);
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr varType = largeProteinParametersClass->getMemberVariableType(i);
      const String varName = largeProteinParametersClass->getMemberVariableName(i);
      if (varType->inheritsFrom(booleanType))
        res[i] = booleanStream(true);
      else if (varName.endsWith(T("WindowSize")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(11);
        values.push_back(15);
        values.push_back(19);
        values.push_back(21);
        values.push_back(25);
        /*
        for (int j = 1; j < 20; j += 2)
          values.push_back(j);
        for (int j = 21; j < 40; j += 4)
          values.push_back(j);
         */
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("LocalHistogramSize")))
      {
        std::vector<int> values;
        for (int j = 10; j < 100; j += 10)
          values.push_back(j);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("separationProfilSize")))
      {
        std::vector<int> values;
        for (int j = 1; j < 20; j += 2)
          values.push_back(j);
        res[i] = integerStream(positiveIntegerType, values);
      }
      if (varName.startsWith(T("cbs")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(11);
        values.push_back(15);
        values.push_back(19);
        values.push_back(21);
        values.push_back(25);
        /*
         for (int j = 1; j < 20; j += 2)
         values.push_back(j);
         for (int j = 21; j < 40; j += 4)
         values.push_back(j);
         */
        res[i] = integerStream(positiveIntegerType, values);
      }
      else
        jassertfalse;
    }
    return res;
  }
  
  static std::vector<StreamPtr> createCbsRelatedStreams()
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<StreamPtr> res(n);
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr varType = largeProteinParametersClass->getMemberVariableType(i);
      const String varName = largeProteinParametersClass->getMemberVariableName(i);
      if (varName.startsWith(T("cbs")))
      {
        std::vector<int> values;
        values.push_back(1);
        values.push_back(5);
        values.push_back(9);
        values.push_back(11);
        values.push_back(15);
        values.push_back(19);
        values.push_back(21);
        values.push_back(25);
        /*
         for (int j = 1; j < 20; j += 2)
         values.push_back(j);
         for (int j = 21; j < 40; j += 4)
         values.push_back(j);
         */
        res[i] = integerStream(positiveIntegerType, values);
      }
    }
    return res;
  }

  static std::vector<StreamPtr> createResidueStreams()
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    const size_t startIndex = largeProteinParametersClass->findMemberVariable(T("usePositionDifference"));

    std::vector<StreamPtr> res = createStreams();
    jassertfalse(res.size() == n);
    for (size_t i = startIndex; i < n; ++i)
      res[i] = StreamPtr();
    return res;
  }

  static std::vector<SamplerPtr> createSamplers()
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<SamplerPtr> res(n);
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr varType = largeProteinParametersClass->getMemberVariableType(i);
      const String varName = largeProteinParametersClass->getMemberVariableName(i);
      if (varType->inheritsFrom(booleanType))
        res[i] = bernoulliSampler(0.25, 0.1, 0.9);
      else if (varName.endsWith(T("WindowSize")))
        res[i] = discretizeSampler(zeroOrScalarContinuousSampler(bernoulliSampler(0.25, 0.1, 0.9), gaussianSampler(15, 15)), 0, 40);
      else if (varName.endsWith(T("LocalHistogramSize")))
        res[i] = discretizeSampler(zeroOrScalarContinuousSampler(bernoulliSampler(0.25, 0.1, 0.9), gaussianSampler(50, 50)), 0, 100);
      else if (varName == T("separationProfilSize"))
        res[i] = discretizeSampler(zeroOrScalarContinuousSampler(bernoulliSampler(0.25, 0.1, 0.9), gaussianSampler(10, 10)), 0, 20);
      else
        jassertfalse;
    }
    return res;
  }

  static std::vector<StreamPtr> createSingleTaskStreams(ProteinTarget target)
  {
    std::vector<StreamPtr> res = createStreams();
    if (target != ss3Target)
    {
      res[largeProteinParametersClass->findMemberVariable(T("useSS3GlobalHistogram"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("ss3WindowSize"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("ss3LocalHistogramSize"))] = StreamPtr();
    }
    if (target != ss8Target)
    {
      res[largeProteinParametersClass->findMemberVariable(T("useSS8GlobalHistogram"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("ss8WindowSize"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("ss8LocalHistogramSize"))] = StreamPtr();
    }
    if (target != sa20Target)
    {
      res[largeProteinParametersClass->findMemberVariable(T("useSAGlobalHistogram"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("saWindowSize"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("saLocalHistogramSize"))] = StreamPtr();
    }
    if (target != drTarget)
    {
      res[largeProteinParametersClass->findMemberVariable(T("useDRGlobalHistogram"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("drWindowSize"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("drLocalHistogramSize"))] = StreamPtr();
    }
    if (target != stalTarget)
    {
      res[largeProteinParametersClass->findMemberVariable(T("useSTALGlobalHistogram"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("stalWindowSize"))] = StreamPtr();
      res[largeProteinParametersClass->findMemberVariable(T("stalLocalHistogramSize"))] = StreamPtr();
    }
    return res;
  }

  static std::vector<StreamPtr> createSingleTaskSingleStageStreams()
  {
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    std::vector<StreamPtr> res(n);
    
    res[largeProteinParametersClass->findMemberVariable(T("useProteinLength"))] = booleanStream(true);
    res[largeProteinParametersClass->findMemberVariable(T("useNumCysteins"))] = booleanStream(true);

    res[largeProteinParametersClass->findMemberVariable(T("useAminoAcidGlobalHistogram"))] = booleanStream(true);
    res[largeProteinParametersClass->findMemberVariable(T("usePSSMGlobalHistogram"))] = booleanStream(true);

    res[largeProteinParametersClass->findMemberVariable(T("useRelativePosition"))] = booleanStream(true);
    res[largeProteinParametersClass->findMemberVariable(T("useRelativeCysteinIndex"))] = booleanStream(true);

    
    std::vector<int> windowValues;
    windowValues.push_back(1);
    windowValues.push_back(5);
    windowValues.push_back(9);
    windowValues.push_back(11);
    windowValues.push_back(15);
    windowValues.push_back(19);
    windowValues.push_back(21);
    windowValues.push_back(25);

    res[largeProteinParametersClass->findMemberVariable(T("aminoAcidWindowSize"))] = integerStream(positiveIntegerType, windowValues);
    res[largeProteinParametersClass->findMemberVariable(T("pssmWindowSize"))] = integerStream(positiveIntegerType, windowValues);

    std::vector<int> localValues;
    for (int j = 10; j < 100; j += 10)
      localValues.push_back(j);

    res[largeProteinParametersClass->findMemberVariable(T("aminoAcidLocalHistogramSize"))] = integerStream(positiveIntegerType, localValues);
    res[largeProteinParametersClass->findMemberVariable(T("pssmLocalHistogramSize"))] = integerStream(positiveIntegerType, localValues);

    std::vector<int> cysSepProfilvalues;
    for (int j = 1; j < 20; j += 2)
      cysSepProfilvalues.push_back(j);
    res[largeProteinParametersClass->findMemberVariable(T("separationProfilSize"))] = integerStream(positiveIntegerType, cysSepProfilvalues);

    return res;
  }

  static std::vector<SamplerPtr> createSingleTaskSingleStageSamplers()
  {
    std::vector<SamplerPtr> res = createSamplers();

    res[largeProteinParametersClass->findMemberVariable(T("useSS3GlobalHistogram"))] = constantSampler(false);
    res[largeProteinParametersClass->findMemberVariable(T("ss3WindowSize"))] = constantSampler(Variable(0, positiveIntegerType));
    res[largeProteinParametersClass->findMemberVariable(T("ss3LocalHistogramSize"))] = constantSampler(Variable(0, positiveIntegerType));

    res[largeProteinParametersClass->findMemberVariable(T("useSS8GlobalHistogram"))] = constantSampler(false);
    res[largeProteinParametersClass->findMemberVariable(T("ss8WindowSize"))] = constantSampler(Variable(0, positiveIntegerType));
    res[largeProteinParametersClass->findMemberVariable(T("ss8LocalHistogramSize"))] = constantSampler(Variable(0, positiveIntegerType));

    res[largeProteinParametersClass->findMemberVariable(T("useSAGlobalHistogram"))] = constantSampler(false);
    res[largeProteinParametersClass->findMemberVariable(T("saWindowSize"))] = constantSampler(Variable(0, positiveIntegerType));
    res[largeProteinParametersClass->findMemberVariable(T("saLocalHistogramSize"))] = constantSampler(Variable(0, positiveIntegerType));

    res[largeProteinParametersClass->findMemberVariable(T("useDRGlobalHistogram"))] = constantSampler(false);
    res[largeProteinParametersClass->findMemberVariable(T("drWindowSize"))] = constantSampler(Variable(0, positiveIntegerType));
    res[largeProteinParametersClass->findMemberVariable(T("drLocalHistogramSize"))] = constantSampler(Variable(0, positiveIntegerType));

    res[largeProteinParametersClass->findMemberVariable(T("useSTALGlobalHistogram"))] = constantSampler(false);
    res[largeProteinParametersClass->findMemberVariable(T("stalWindowSize"))] = constantSampler(Variable(0, positiveIntegerType));
    res[largeProteinParametersClass->findMemberVariable(T("stalLocalHistogramSize"))] = constantSampler(Variable(0, positiveIntegerType));

    return res;
  }

  static LargeProteinParametersPtr createTestObject(size_t windowSizes = 3)
  {
    LargeProteinParametersPtr res = new LargeProteinParameters();
    const size_t n = largeProteinParametersClass->getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr varType = largeProteinParametersClass->getMemberVariableType(i);
      const String varName = largeProteinParametersClass->getMemberVariableName(i);
      if (varType->inheritsFrom(booleanType))
        res->setVariable(i, Variable(true, booleanType));
      else
        res->setVariable(i, Variable(windowSizes, positiveIntegerType));
    }
    return res;
  }

protected:
  friend class LargeProteinParametersClass;
};

class DumbBinaryClassifierFunction : public Function
{
public:
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? anyType : getSupervisionType();}

  virtual String getOutputPostFix() const
    {return T("Dumb");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return probability(0.f);}
};

/*
** Large Protein Predictor Parameters
*/
class LargeProteinPredictorParameters : public CFProteinPredictorParameters
{
public:
  LargeProteinPredictorParameters(const LargeProteinParametersPtr& fp = LargeProteinParametersPtr(), double oxidizedCysteineThreshold = 0.5f)
    : CFProteinPredictorParameters(oxidizedCysteineThreshold)
    , fp(fp), learningMachineName(T("SGD"))
    , svmC(4.0), svmGamma(1.0)
    , knnNeighbors(1)
    , x3Trees(1000)
    , x3Attributes(0)
    , x3Splits(0)
    , sgdRate(1.0)
    , sgdIterations(100)
    , useAddBias(false)
    , useFisherFilter(false)
    , numFisherFeatures(100)
    , useNormalization(false)
  {}

  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();

      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(new ProteinLengthFunction(), protein);
      builder.addFunction(new NumCysteinsFunction(), protein);
      // AA
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, aaResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // PSSM
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, pssmResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SS3
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss3ResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SS8
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss8ResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // SA
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, saResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // DR
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, drResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // STAL
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, stalResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);

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

  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t numCysteins = builder.addFunction(getVariableFunction(T("numCysteins")), proteinPerception, T("#Cys"));
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception, T("length"));
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccu"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccu"));
    size_t ss3Accumulator = builder.addFunction(getVariableFunction(T("ss3Accumulator")), proteinPerception, T("ss3Accu"));
    size_t ss8Accumulator = builder.addFunction(getVariableFunction(T("ss8Accumulator")), proteinPerception, T("ss8Accu"));
    size_t saAccumulator = builder.addFunction(getVariableFunction(T("saAccumulator")), proteinPerception, T("saAccu"));
    size_t drAccumulator = builder.addFunction(getVariableFunction(T("drAccumulator")), proteinPerception, T("drAccu"));
    size_t stalAccumulator = builder.addFunction(getVariableFunction(T("stalAccumulator")), proteinPerception, T("stalAccu"));
    /* Output */
    builder.startSelection();
      /*** Global Features ***/
      if (fp->useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));
      
      // global histograms
      if (fp->useAminoAcidGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      if (fp->usePSSMGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));
      if (fp->useSS3GlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), ss3Accumulator, T("h(SS3)"));
      if (fp->useSS8GlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), ss8Accumulator, T("h(SS8)"));
      if (fp->useSAGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), saAccumulator, T("h(SA)"));
      if (fp->useDRGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), drAccumulator, T("h(DR)"));
      if (fp->useSTALGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), stalAccumulator, T("h(StAl)"));
      
      // number of cysteins
      if (fp->useNumCysteins)
      {
        builder.addFunction(integerFeatureGenerator(), numCysteins, T("#Cys"));
        builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));
      }

      // bias (and anti-crash)
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Output */
    builder.startSelection();
      builder.addFunction(getVariableFunction(T("length")), proteinPerception);
      builder.addInSelection(proteinPerception);
    builder.finishSelectionWithFunction(createVectorFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, residuePerception)), T("rfVector"));
  }

  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Output */
    builder.startSelection();
      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(LargeProteinPredictorParameters, cysteinResiduePairVectorFeatures), 0.f)
                                        , T("cysteinResiduePairFeatures"));
  }

  virtual void oxidizedCysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Output */
    builder.startSelection();
      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);
    
    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
                                        lbcppMemberCompositeFunction(LargeProteinPredictorParameters, cysteinResiduePairVectorFeatures), getOxidizedCysteineThreshold())
                                        , T("oxidizedCysteinResiduePairFeatures"));
  }

  void cysteinResiduePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType);
    size_t secondPosition = builder.addInput(positiveIntegerType);
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);

    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccu"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccu"));
    size_t ss3Accumulator = builder.addFunction(getVariableFunction(T("ss3Accumulator")), proteinPerception, T("ss3Accu"));
    size_t ss8Accumulator = builder.addFunction(getVariableFunction(T("ss8Accumulator")), proteinPerception, T("ss8Accu"));
    size_t saAccumulator = builder.addFunction(getVariableFunction(T("saAccumulator")), proteinPerception, T("saAccu"));
    size_t drAccumulator = builder.addFunction(getVariableFunction(T("drAccumulator")), proteinPerception, T("drAccu"));
    size_t stalAccumulator = builder.addFunction(getVariableFunction(T("stalAccumulator")), proteinPerception, T("stalAccu"));

    size_t cysDist = builder.addFunction(new SubtractFunction(), secondIndex, firstIndex, T("|CYS2-CYS1|"));
    size_t aaDist = builder.addFunction(new SubtractFunction(), secondPosition, firstPosition, T("|AA2-AA1|"));
    
    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("dsb"));
    /* Output */
    builder.startSelection();
      builder.addFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, residuePerception), firstPosition, proteinPerception, T("rf1"));
      builder.addFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, residuePerception), secondPosition, proteinPerception, T("rf2"));

    if (fp->useCysteinDistance)
      builder.addFunction(doubleFeatureGenerator(), cysDist, T("|CYS2-CYS1|"));
    if (fp->useAADistance)
      builder.addFunction(doubleFeatureGenerator(), aaDist, T("|AA2-AA1|"));

    if (fp->usePositionDifference)
      builder.addFunction(new NormalizedCysteinPositionDifference(), protein, firstPosition, secondPosition, T("NCPD"));
    if (fp->useIndexDifference)
      builder.addFunction(new NormalizedCysteinIndexDifference(), protein, firstIndex, secondIndex, T("NCID"));

    if (fp->useAminoAcidIntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), aaAccumulator, firstPosition, secondPosition, T("h(AA1,AA2)"));
    if (fp->usePSSMIntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), pssmAccumulator, firstPosition, secondPosition, T("h(PSSM1,PSSM2)"));
    if (fp->useSS3IntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), ss3Accumulator, firstPosition, secondPosition, T("h(SS3_1,SS3_2)"));
    if (fp->useSS8IntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), ss8Accumulator, firstPosition, secondPosition, T("h(SS8_1,SS8_2)"));
    if (fp->useSAIntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), saAccumulator, firstPosition, secondPosition, T("h(SA1,SA2)"));
    if (fp->useDRIntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), drAccumulator, firstPosition, secondPosition, T("h(DR1,DR2)"));
    if (fp->useSTALIntervalHistogram)
      builder.addFunction(accumulatorWindowMeanFunction(), stalAccumulator, firstPosition, secondPosition, T("h(STAL1,STAL2)"));

    if (fp->useDsbProbability || fp->useNormDsbProbability)
      builder.addFunction(new DisulfideInfoFeatureGenerator(fp->useDsbProbability, fp->useNormDsbProbability), dsb, firstIndex, secondIndex, T("dsb"));

    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);
    
    builder.finishSelectionWithFunction(new CreateCysteinBondingStateVectorFunction(
                                         lbcppMemberCompositeFunction(LargeProteinPredictorParameters, residuePerception))
                                        , T("cysteinBondingStateResidueFeature"));    
    
  }

  void residueFeatures(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  virtual void residuePerception(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);
    size_t numCysteins = builder.addFunction(getVariableFunction(T("numCysteins")), proteinPerception, T("#Cys"));
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception, T("length"));
    size_t aaResidueFeatures = builder.addFunction(getVariableFunction(T("aaResidueFeatures")), proteinPerception, T("pssmRF"));
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccu"));
    size_t pssmResidueFeatures = builder.addFunction(getVariableFunction(T("pssmResidueFeatures")), proteinPerception, T("pssmRF"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccu"));
    size_t ss3ResidueFeatures = builder.addFunction(getVariableFunction(T("ss3ResidueFeatures")), proteinPerception, T("ss3RF"));
    size_t ss3Accumulator = builder.addFunction(getVariableFunction(T("ss3Accumulator")), proteinPerception, T("ss3Accu"));
    size_t ss8ResidueFeatures = builder.addFunction(getVariableFunction(T("ss8ResidueFeatures")), proteinPerception, T("ss8RF"));
    size_t ss8Accumulator = builder.addFunction(getVariableFunction(T("ss8Accumulator")), proteinPerception, T("ss8Accu"));
    size_t saResidueFeatures = builder.addFunction(getVariableFunction(T("saResidueFeatures")), proteinPerception, T("saRF"));
    size_t saAccumulator = builder.addFunction(getVariableFunction(T("saAccumulator")), proteinPerception, T("saAccu"));
    size_t drResidueFeatures = builder.addFunction(getVariableFunction(T("drResidueFeatures")), proteinPerception, T("drRF"));
    size_t drAccumulator = builder.addFunction(getVariableFunction(T("drAccumulator")), proteinPerception, T("drAccu"));
    size_t stalResidueFeatures = builder.addFunction(getVariableFunction(T("stalResidueFeatures")), proteinPerception, T("stalRF"));
    size_t stalAccumulator = builder.addFunction(getVariableFunction(T("stalAccumulator")), proteinPerception, T("stalAccu"));

    size_t cbsVector = builder.addFunction(getVariableFunction(T("cysteinBondingStates")), protein, T("cbs"));
    size_t idCbsVector = builder.addFunction(new NormalizeDenseDoubleVector(NormalizeDenseDoubleVector::none), cbsVector);
    size_t relativeCbsVector = builder.addFunction(new NormalizeDenseDoubleVector(NormalizeDenseDoubleVector::relative), cbsVector);
    size_t meanCbsVector = builder.addFunction(new NormalizeDenseDoubleVector(NormalizeDenseDoubleVector::mean), cbsVector);
    size_t stdDevCbsVector = builder.addFunction(new NormalizeDenseDoubleVector(NormalizeDenseDoubleVector::meanAndStandardDeviation), cbsVector);

    size_t dsb = builder.addFunction(getVariableFunction(T("disulfideBonds")), protein, T("dsb"));

    /* Output */
    builder.startSelection();
      /*** Global Features ***/
      if (fp->useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));

      // global histograms
      if (fp->useAminoAcidGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));
      if (fp->usePSSMGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));
      if (fp->useSS3GlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), ss3Accumulator, T("h(SS3)"));
      if (fp->useSS8GlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), ss8Accumulator, T("h(SS8)"));
      if (fp->useSAGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), saAccumulator, T("h(SA)"));
      if (fp->useDRGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), drAccumulator, T("h(DR)"));
      if (fp->useSTALGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), stalAccumulator, T("h(StAl)"));

    if (fp->cbsAbsoluteSize)
      builder.addFunction(centeredContainerWindowFeatureGenerator(fp->cbsAbsoluteSize), idCbsVector, cysteinIndex, T("w(CBS_abs,") + String((int)fp->cbsAbsoluteSize) + (")"));
    if (fp->cbsRelativeSize)
      builder.addFunction(centeredContainerWindowFeatureGenerator(fp->cbsRelativeSize), relativeCbsVector, cysteinIndex, T("w(CBS_rel,") + String((int)fp->cbsRelativeSize) + (")"));
    if (fp->cbsMeanSize)
      builder.addFunction(centeredContainerWindowFeatureGenerator(fp->cbsMeanSize), meanCbsVector, cysteinIndex, T("w(CBS_mean,") + String((int)fp->cbsMeanSize) + (")"));
    if (fp->cbsStdDevSize)
      builder.addFunction(centeredContainerWindowFeatureGenerator(fp->cbsStdDevSize), stdDevCbsVector, cysteinIndex, T("w(CBS_std,") + String((int)fp->cbsStdDevSize) + (")"));

      // number of cysteins
      if (fp->useNumCysteins)
      {
        builder.addFunction(integerFeatureGenerator(), numCysteins, T("#Cys"));
        builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));
      }

      // bias (and anti-crash)
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));

      /*** Residue Features ***/
      if (fp->useRelativePosition)
        builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("Pos/Len"));
      if (fp->useRelativeCysteinIndex)
        builder.addFunction(new RelativeValueFeatureGenerator(1), cysteinIndex, numCysteins, T("Cys/#Cys"));

      // window sizes
      if (fp->aminoAcidWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->aminoAcidWindowSize), aaResidueFeatures, position, T("w(AA,") + String((int)fp->aminoAcidWindowSize) + (")"));
      if (fp->pssmWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->pssmWindowSize), pssmResidueFeatures, position, T("w(PSSM,") + String((int)fp->pssmWindowSize) + (")"));
      if (fp->ss3WindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->ss3WindowSize), ss3ResidueFeatures, position, T("w(SS3,") + String((int)fp->ss3WindowSize) + (")"));
      if (fp->ss8WindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->ss8WindowSize), ss8ResidueFeatures, position, T("w(SS8,") + String((int)fp->ss8WindowSize) + (")"));
      if (fp->saWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->saWindowSize), saResidueFeatures, position, T("w(SA,") + String((int)fp->saWindowSize) + (")"));
      if (fp->drWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->drWindowSize), drResidueFeatures, position, T("w(DR,") + String((int)fp->drWindowSize) + (")"));
      if (fp->stalWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->stalWindowSize), stalResidueFeatures, position, T("w(StAl,") + String((int)fp->stalWindowSize) + (")"));

      // local histogram
      if (fp->aminoAcidLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->aminoAcidLocalHistogramSize), aaAccumulator, position, T("h(AA,") + String((int)fp->aminoAcidLocalHistogramSize) + T(")"));
      if (fp->pssmLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->pssmLocalHistogramSize), pssmAccumulator, position, T("h(PSSM,") + String((int)fp->pssmLocalHistogramSize) + T(")"));
      if (fp->ss3LocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->ss3LocalHistogramSize), ss3Accumulator, position, T("h(SS3,") + String((int)fp->ss3LocalHistogramSize) + T(")"));
      if (fp->ss8LocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->ss8LocalHistogramSize), ss8Accumulator, position, T("h(SS8,") + String((int)fp->ss8LocalHistogramSize) + T(")"));
      if (fp->saLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->saLocalHistogramSize), saAccumulator, position, T("h(SA,") + String((int)fp->saLocalHistogramSize) + T(")"));
      if (fp->drLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->drLocalHistogramSize), drAccumulator, position, T("h(DR,") + String((int)fp->drLocalHistogramSize) + T(")"));
      if (fp->stalLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->stalLocalHistogramSize), stalAccumulator, position, T("h(StAl,") + String((int)fp->stalLocalHistogramSize) + T(")"));

      // cystein profil
      if (fp->separationProfilSize)
        builder.addFunction(new CysteinSeparationProfilFeatureGenerator(fp->separationProfilSize, true), protein, position, T("CysProfil(") + String((int)fp->separationProfilSize) + T(")"));
    if (fp->dsbProfilSize)
      builder.addFunction(new DisuflideSeparationProfilFeatureGenerator(fp->dsbProfilSize, false), dsb, cysteinIndex, T("DsbProfil(") + String((int)fp->separationProfilSize) + T(")"));
    if (fp->normalizedDsbProfilSize)
      builder.addFunction(new DisuflideSeparationProfilFeatureGenerator(fp->normalizedDsbProfilSize, true), dsb, cysteinIndex, T("NormDsbProfil(") + String((int)fp->separationProfilSize) + T(")"));
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
/*
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    if (target == dsbTarget && useAddBias)
    {
      FunctionPtr res = learningMachine(target);
      if (useFisherFilter)
        res = new PreProcessInputCompositeFunction(fisherFilterLearnableFunction(numFisherFeatures), res);
      return new ConnectivityPatternClassifier(res, target == dsbTarget);
    }

    FunctionPtr res = ProteinPredictorParameters::createTargetPredictor(target);
    if (useFisherFilter)
      return new PreProcessInputCompositeFunction(fisherFilterLearnableFunction(numFisherFeatures), res);
    return res;
  }
*/
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    if (learner)
      return learner;
    
    // TODO: incorporate bias in case of binary target
    if (learningMachineName == T("LibSVM"))
      return new PreProcessInputCompositeFunction(doubleVectorNormalizeFunction(true, true)
                                                  , libSVMLearningMachine(pow(2.0, svmC), rbfKernel, 0, pow(2.0, svmGamma), 0.0));
    else if (learningMachineName == T("kNN"))
    {
      FunctionPtr res = nearestNeighborLearningMachine(knnNeighbors, true);
      if (useNormalization)
        res = new PreProcessInputCompositeFunction(composeFunction(
                                                                   doubleVectorNormalizeFunction(true, true), concatenatedDoubleVectorNormalizeFunction()
                                                                   ), res);
      return res;
    }
    else if (learningMachineName == T("LSH"))
      return binaryLocalitySensitiveHashing(knnNeighbors);
    else if (learningMachineName == T("ExtraTrees"))
    {
      FunctionPtr res = extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits);
      if (useAddBias)
        res = new PreProcessCompositeFunction(res, composeFunction(addBiasLearnableFunction(binaryClassificationSensitivityAndSpecificityScore, 0.0, true)
                                                                   , signedScalarToProbabilityFunction()));
      return res;
    }
    else if (learningMachineName == T("SGD"))
    {
      FunctionPtr res = linearBinaryClassifier(new StochasticGDParameters(constantIterationFunction(sgdRate), StoppingCriterionPtr(), sgdIterations, false, true, true, true, false));
      //linearLearningMachine(new BinaryBalancedStochasticGDParameters(constantIterationFunction(sgdRate), StoppingCriterionPtr(), sgdIterations));
      //res->setEvaluator(defaultSupervisedEvaluator());
      if (useNormalization)
        res = new PreProcessInputCompositeFunction(doubleVectorNormalizeFunction(true, true), res);
      return res;
    }
    else if (learningMachineName == T("kNN-LOO"))
    {
      jassert(target == dsbTarget);
      FunctionPtr res = binaryClassificationStreamBasedNearestNeighbor(knnNeighbors, false);
      res->setBatchLearner(balanceBinaryExamplesBatchLearner(res->getBatchLearner()));
      return res;
    }
    else if (learningMachineName == T("Dumb"))
      return new DumbBinaryClassifierFunction();

    jassertfalse;
    return FunctionPtr();
  }

  void setParameters(LargeProteinParametersPtr parameters)
    {fp = parameters;}

public:
  LargeProteinParametersPtr fp;

  String learningMachineName;
  double svmC;
  double svmGamma;
  size_t knnNeighbors;
  size_t x3Trees;
  size_t x3Attributes;
  size_t x3Splits;
  double sgdRate;
  size_t sgdIterations;

  bool useAddBias;
  bool useFisherFilter;
  size_t numFisherFeatures;
  bool useNormalization;
  
  FunctionPtr learner;

protected:
  friend class LargeProteinPredictorParametersClass;
};

typedef ReferenceCountedObjectPtr<LargeProteinPredictorParameters> LargeProteinPredictorParametersPtr;

};

#endif // !LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LARGE_H_
