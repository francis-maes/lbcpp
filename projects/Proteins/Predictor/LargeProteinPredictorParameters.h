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

namespace lbcpp
{

class ParameteredCompositeFunction : public CompositeFunction
{
public:
  void setParameter(const Variable& v)
    {parameter = v;}
  
protected:
  friend class ParameteredCompositeFunctionClass;

  Variable parameter;
};

typedef ReferenceCountedObjectPtr<ParameteredCompositeFunction> ParameteredCompositeFunctionPtr;

class ProteinDecoratedCreateVector : public CompositeFunction
{
public:
  ProteinDecoratedCreateVector(const FunctionPtr& function)
    : function(function) {}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));
    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    builder.addFunction(createVectorFunction(function), length, protein);
  }

protected:
  friend class ProteinDecoratedCreateVectorClass;

  FunctionPtr function;

  ProteinDecoratedCreateVector() {}
};

class IdentityCreateVector : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t length = builder.addInput(positiveIntegerType, T("length"));
    size_t value = builder.addInput(anyType, T("value"));
    builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(IdentityCreateVector, identity)), length, value);
  }

protected:
  void identity(CompositeFunctionBuilder& builder) const
  {
    builder.addInput(positiveIntegerType);
    builder.addInput(anyType, T("value"));
  }
};

class WindowFunction : public ParameteredCompositeFunction
{
public:
  WindowFunction(const FunctionPtr& residueFunction, const String& residueShortName = T("??"))
  : residueFunction(residueFunction), residueShortName(residueShortName) {}
  
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    size_t primaryFeatures = builder.addFunction(createVectorFunction(residueFunction), length, protein);
    
    builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(WindowFunction, perResidue)), length, primaryFeatures);
  }
protected:
  friend class WindowFunctionClass;

  FunctionPtr residueFunction;
  String residueShortName;
  
  WindowFunction() {}

  void perResidue(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t primaryFeatures = builder.addInput(containerClass(), T("primary"));
    builder.addFunction(centeredContainerWindowFeatureGenerator(parameter.getInteger()), primaryFeatures, position, T("w(") + residueShortName + T(",") + String(parameter.getInteger())+ T(")"));
  }
};

class GlobalHistogramFunction : public ParameteredCompositeFunction
{
public:
  GlobalHistogramFunction(const FunctionPtr& residueFunction, const String& residueShortName = T("??"))
  : residueFunction(residueFunction), residueShortName(residueShortName) {}
  
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    size_t primaryFeatures = builder.addFunction(createVectorFunction(residueFunction), length, protein);
    size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
    size_t result = builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("h(") + residueShortName + (")"));
    
    builder.addFunction(new IdentityCreateVector(), length, result);
  }
  
protected:
  friend class GlobalHistogramFunctionClass;

  FunctionPtr residueFunction;
  String residueShortName;

  GlobalHistogramFunction() {}
};

class LocalHistogramFunction : public ParameteredCompositeFunction
{
public:
  LocalHistogramFunction(const FunctionPtr& residueFunction, const String& residueShortName = T("??"))
  : residueFunction(residueFunction), residueShortName(residueShortName) {}
  
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    size_t primaryFeatures = builder.addFunction(createVectorFunction(residueFunction), length, protein);
    size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
    
    builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(LocalHistogramFunction, perResidue)), length, primaryFeaturesAcc);
  }
  
protected:
  friend class LocalHistogramFunctionClass;

  FunctionPtr residueFunction;
  String residueShortName;

  LocalHistogramFunction() {}
  
  void perResidue(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t primaryFeaturesAcc = builder.addInput(containerClass(), T("accumulator"));
    
    builder.addFunction(accumulatorLocalMeanFunction(parameter.getInteger()), primaryFeaturesAcc, position, T("h(") + residueShortName + T(",") + String(parameter.getInteger()) + T(")"));
  }
};

class SeperationProfilFunction : public ParameteredCompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(SeperationProfilFunction, perResidue)), length, protein, T("residueFeatures"));
  }
  
protected:
  void perResidue(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    builder.addFunction(new CysteinSeparationProfilFeatureGenerator(parameter.getInteger(), true), protein, position, T("CysProfil(") + String(parameter.getInteger()) + T(")"));
  }
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

    separationProfilSize(0)
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
        for (int j = 1; j < 20; j += 2)
          values.push_back(j);
        for (int j = 21; j < 40; j += 4)
          values.push_back(j);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName.endsWith(T("LocalHistogramSize")))
      {
        std::vector<int> values;
        for (int j = 10; j < 100; j += 10)
          values.push_back(j);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("separationProfilSize"))
      {
        std::vector<int> values;
        for (int j = 1; j < 20; j += 2)
          values.push_back(j);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else
        jassertfalse;
    }
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
    std::vector<StreamPtr> res = createStreams();

    res[largeProteinParametersClass->findMemberVariable(T("useSS3GlobalHistogram"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("ss3WindowSize"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("ss3LocalHistogramSize"))] = StreamPtr();

    res[largeProteinParametersClass->findMemberVariable(T("useSS8GlobalHistogram"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("ss8WindowSize"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("ss8LocalHistogramSize"))] = StreamPtr();

    res[largeProteinParametersClass->findMemberVariable(T("useSAGlobalHistogram"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("saWindowSize"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("saLocalHistogramSize"))] = StreamPtr();

    res[largeProteinParametersClass->findMemberVariable(T("useDRGlobalHistogram"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("drWindowSize"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("drLocalHistogramSize"))] = StreamPtr();

    res[largeProteinParametersClass->findMemberVariable(T("useSTALGlobalHistogram"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("stalWindowSize"))] = StreamPtr();
    res[largeProteinParametersClass->findMemberVariable(T("stalLocalHistogramSize"))] = StreamPtr();

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

protected:
  friend class LargeProteinParametersClass;
};

/*
** Large Protein Predictor Parameters
*/
class LargeProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  LargeProteinPredictorParameters(const LargeProteinParametersPtr& fp, bool isGlobalFeaturesLazy = false)
    : fp(fp), isGlobalFeaturesLazy(isGlobalFeaturesLazy), learningMachineName(T("SGD"))
    , svmC(4.0), svmGamma(1.0)
    , knnNeighbors(5)
    , x3Trees(1000)
    , x3Attributes(0)
    , x3Splits(0)
    , sgdRate(1.0)
    , sgdIterations(100)
    , useAddBias(false)
  {
    initializeFeatureGenerators();
  }

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

  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Input */
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass(), T("proteinPerception"));
    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception);
    size_t numCys = builder.addFunction(getVariableFunction(T("numCysteins")), proteinPerception);    
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccu"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccu"));
    size_t ss3Accumulator = builder.addFunction(getVariableFunction(T("ss3Accumulator")), proteinPerception, T("ss3Accu"));
    size_t ss8Accumulator = builder.addFunction(getVariableFunction(T("ss8Accumulator")), proteinPerception, T("ss8Accu"));
    size_t saAccumulator = builder.addFunction(getVariableFunction(T("saAccumulator")), proteinPerception, T("saAccu"));
    size_t drAccumulator = builder.addFunction(getVariableFunction(T("drAccumulator")), proteinPerception, T("drAccu"));
    size_t stalAccumulator = builder.addFunction(getVariableFunction(T("stalAccumulator")), proteinPerception, T("stalAccu"));
    /* Output */
    builder.startSelection();
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
        builder.addFunction(integerFeatureGenerator(), numCys, T("#Cys"));
        builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));
      }

      // bias (and anti-crash)
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(isGlobalFeaturesLazy));
  }

  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

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
    {jassertfalse;}

  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

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

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    if (target == dsbTarget && useAddBias)
      return new ConnectivityPatternClassifier(learningMachine(target), target == dsbTarget);
    return ProteinPredictorParameters::createTargetPredictor(target);
  }

  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    // TODO: incorporate bias in case of binary target
    if (learningMachineName == T("LibSVM"))
      return libSVMLearningMachine(pow(2.0, svmC), rbfKernel, 0, pow(2.0, svmGamma), 0.0);
    else if (learningMachineName == T("kNN"))
      return nearestNeighborLearningMachine(knnNeighbors, false);
    else if (learningMachineName == T("LSH"))
      return binaryLocalitySensitiveHashing(knnNeighbors);
    else if (learningMachineName == T("ExtraTrees"))
      return extraTreeLearningMachine(x3Trees, x3Attributes, x3Splits);
    else if (learningMachineName == T("SGD"))
    {
      FunctionPtr res = linearLearningMachine(new StochasticGDParameters(constantIterationFunction(sgdRate), StoppingCriterionPtr(), sgdIterations));
      res->setEvaluator(defaultSupervisedEvaluator());
      return res;
    }

    jassertfalse;
    return FunctionPtr();
  }

  void setParameters(LargeProteinParametersPtr parameters)
    {fp = parameters;}

  void proteinLengthFeatures(CompositeFunctionBuilder& builder) const
  {
    builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass, T("protein"));

    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));
    builder.addFunction(integerFeatureGenerator(), length, T("length"));
  }

  void numCysteinFeatures(CompositeFunctionBuilder& builder) const
  {
    builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass, T("protein"));

    size_t numCys = builder.addFunction(new NumCysteinsFunction(), protein);
    builder.addFunction(integerFeatureGenerator(), numCys, T("#Cys"));
  }

  void isNumCysteinEven(CompositeFunctionBuilder& builder) const
  {
    builder.addInput(positiveIntegerType);
    size_t protein = builder.addInput(proteinClass, T("protein"));

    builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));
  }

  void relativePositionFeature(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    size_t length = builder.addFunction(new ProteinLengthFunction(), protein, T("length"));

    builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("Pos/Len"));
  }

  void relativeCysteinIndexFeature(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);
    size_t numCysteins = builder.addFunction(new NumCysteinsFunction(), protein);

    builder.addFunction(new RelativeValueFeatureGenerator(1), cysteinIndex, numCysteins, T("Cys/#Cys"));
  }

  void initializeFeatureGenerators()
  {
    featureGenerators.resize(largeProteinParametersClass->getNumMemberVariables());
    addFeatureGenerator(T("useProteinLength"),    new ProteinDecoratedCreateVector(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, proteinLengthFeatures)));
    addFeatureGenerator(T("useNumCysteins"),      new ProteinDecoratedCreateVector(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, numCysteinFeatures)));
    addFeatureGenerator(T("useIsNumCysteinEven"), new ProteinDecoratedCreateVector(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, isNumCysteinEven)));

    addFeatureGenerator(T("useAminoAcidGlobalHistogram"), new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, aaResidueFeatures), T("AA")));
    addFeatureGenerator(T("usePSSMGlobalHistogram"),      new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, pssmResidueFeatures), T("PSSM")));
    addFeatureGenerator(T("useSS3GlobalHistogram"),       new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss3ResidueFeatures), T("SS3")));
    addFeatureGenerator(T("useSS8GlobalHistogram"),       new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss8ResidueFeatures), T("SS8")));
    addFeatureGenerator(T("useSAGlobalHistogram"),        new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, saResidueFeatures), T("SA")));
    addFeatureGenerator(T("useDRGlobalHistogram"),        new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, drResidueFeatures), T("DR")));
    addFeatureGenerator(T("useSTALGlobalHistogram"),      new GlobalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, stalResidueFeatures), T("StAl")));

    addFeatureGenerator(T("useRelativePosition"),     new ProteinDecoratedCreateVector(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, relativePositionFeature)));
    addFeatureGenerator(T("useRelativeCysteinIndex"), new ProteinDecoratedCreateVector(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, relativeCysteinIndexFeature)));

    addFeatureGenerator(T("aminoAcidWindowSize"), new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, aaResidueFeatures), T("AA")));
    addFeatureGenerator(T("pssmWindowSize"),      new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, pssmResidueFeatures), T("PSSM")));
    addFeatureGenerator(T("ss3WindowSize"),       new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss3ResidueFeatures), T("SS3")));
    addFeatureGenerator(T("ss8WindowSize"),       new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss8ResidueFeatures), T("SS8")));
    addFeatureGenerator(T("saWindowSize"),        new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, saResidueFeatures), T("SA")));
    addFeatureGenerator(T("drWindowSize"),        new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, drResidueFeatures), T("DR")));
    addFeatureGenerator(T("stalWindowSize"),      new WindowFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, stalResidueFeatures), T("StAl")));
    
    addFeatureGenerator(T("aminoAcidLocalHistogramSize"), new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, aaResidueFeatures), T("AA")));
    addFeatureGenerator(T("pssmLocalHistogramSize"),      new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, pssmResidueFeatures), T("PSSM")));
    addFeatureGenerator(T("ss3LocalHistogramSize"),       new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss3ResidueFeatures), T("SS3")));
    addFeatureGenerator(T("ss8LocalHistogramSize"),       new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, ss8ResidueFeatures), T("SS8")));
    addFeatureGenerator(T("saLocalHistogramSize"),        new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, saResidueFeatures), T("SA")));
    addFeatureGenerator(T("drLocalHistogramSize"),        new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, drResidueFeatures), T("DR")));
    addFeatureGenerator(T("stalLocalHistogramSize"),      new LocalHistogramFunction(lbcppMemberCompositeFunction(LargeProteinPredictorParameters, stalResidueFeatures), T("StAl")));

    addFeatureGenerator(T("separationProfilSize"), new SeperationProfilFunction());
  }

  ContainerPtr computeFeatures(ExecutionContext& context, size_t variableIndex, Variable value, ContainerPtr proteins) //const
  {
    if (featureGenerators[variableIndex] == FunctionPtr())
    {
      jassertfalse;
      return ContainerPtr();
    }

    if (value.isBoolean() && !value.getBoolean())
      return ContainerPtr();
    
    if (value.isInteger() && value.getInteger() == 0)
      return ContainerPtr();

    if (featureGenerators[variableIndex].dynamicCast<ParameteredCompositeFunction>())
      featureGenerators[variableIndex].dynamicCast<ParameteredCompositeFunction>()->setParameter(value);
    featureGenerators[variableIndex]->initialize(context, proteinClass);

    const size_t n = proteins->getNumElements();
    VectorPtr res = vector(featureGenerators[variableIndex]->getOutputType()->getTemplateArgument(0));
    for (size_t i = 0; i < n; ++i)
    {
      ContainerPtr features = featureGenerators[variableIndex]->compute(context, proteins->getElement(i).getObject()->getVariable(0)).getObjectAndCast<Container>();
      const size_t numFeatures = features->getNumElements();
      //std::cout << "ComputeFeature: " << features->getClass()->toString() << std::endl;
      for (size_t j = 0; j < numFeatures; ++j)
        res->append(features->getElement(j));
    }
    return res;
  }

  void addFeatureGenerator(const String& varName, const FunctionPtr& f)
  {
    const int index = largeProteinParametersClass->findMemberVariable(varName);
    if (index < 0)
      return;
    featureGenerators[index] = f;
  }

  virtual FunctionPtr createResidueVectorPerception() const
  {
    FunctionPtr res = ProteinPredictorParameters::createResidueVectorPerception();
    return composeFunction(res, mapContainerFunction(doubleVectorNormalizeFunction(true, false)));
  }

public:
  LargeProteinParametersPtr fp;
  bool isGlobalFeaturesLazy;

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

  LargeProteinPredictorParameters()
  : svmC(4.0), svmGamma(1.0)
  , knnNeighbors(5)
  , x3Trees(1000)
  , x3Attributes(0)
  , x3Splits(0)
  , sgdRate(1.0)
  , sgdIterations(100)
  , useAddBias(false)
  {
    initializeFeatureGenerators();
  }

protected:
  friend class LargeProteinPredictorParametersClass;

private:
  std::vector<FunctionPtr> featureGenerators;

};

typedef ReferenceCountedObjectPtr<LargeProteinPredictorParameters> LargeProteinPredictorParametersPtr;

};

#endif // !LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LARGE_H_
