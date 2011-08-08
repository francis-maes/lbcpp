/*-----------------------------------------.---------------------------------.
| Filename: Lin09PredictorParameters.h     | Numerical Predictor             |
| Author  : Julien Becker                  |  Parameters from Lin et al. 09  |
| Started : 11/06/2011 10:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_
# define LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_

# include "ProteinPredictor.h"
# include "ProteinPerception.h"
# include "ProteinPredictorParameters.h"
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/Learning/NearestNeighbor.h>
# include <lbcpp/Learning/DecisionTree.h>

namespace lbcpp
{

extern ClassPtr lin09ProteinPerceptionClass(TypePtr, TypePtr, TypePtr);
extern ClassPtr lin09PredictorParametersClass;

class Lin09ProteinPerception : public ProteinPrimaryPerception
{
public:
  DoubleVectorPtr globalFeatures;

  VectorPtr aaResidueFeatures;
  ContainerPtr aaAccumulator;

  VectorPtr pssmResidueFeatures;
  ContainerPtr pssmAccumulator;

  Lin09ProteinPerception(TypePtr globalType, TypePtr aaType, TypePtr pssmType)
    : ProteinPrimaryPerception(lin09ProteinPerceptionClass(globalType, aaType, pssmType)) {}
  Lin09ProteinPerception(TypePtr thisClass)
    : ProteinPrimaryPerception(thisClass) {}
  Lin09ProteinPerception() {}

protected:
  friend class Lin09ProteinPerceptionClass;
};

typedef ReferenceCountedObjectPtr<Lin09ProteinPerception> Lin09ProteinPerceptionPtr;

class CreateLin09ProteinPerception : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return lin09ProteinPerceptionClass(anyType, anyType, anyType)->getNumMemberVariables();}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return lin09ProteinPerceptionClass(anyType, anyType, anyType)->getMemberVariableType(index);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr globalType = inputVariables[3]->getType()->getTemplateArgument(0);
    TypePtr aaType = inputVariables[4]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    TypePtr pssmType = inputVariables[6]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    return lin09ProteinPerceptionClass(globalType, aaType, pssmType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    Lin09ProteinPerceptionPtr res = new Lin09ProteinPerception(getOutputType());
    const size_t n = getNumInputs();
    for (size_t i = 0; i < n; ++i)
      res->setVariable(i, inputs[i]);
    return res;
  }
};

class UpdateGlobalFeatureProteinPerception : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? (TypePtr)doubleVectorClass(enumValueType, doubleType)
                  : (TypePtr)lin09ProteinPerceptionClass(anyType, anyType, anyType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr globalType = inputVariables[1]->getType()->getTemplateArgument(0);
    const TypePtr inputType = inputVariables[0]->getType();
    return lin09ProteinPerceptionClass(globalType, inputType->getTemplateArgument(1), inputType->getTemplateArgument(2));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    Lin09ProteinPerceptionPtr res = inputs[0].getObjectAndCast<Lin09ProteinPerception>(context);
    res->globalFeatures = inputs[1].getObjectAndCast<DoubleVector>(context);
    res->setThisClass(getOutputType());
    return res;
  }
};

class Lin09Parameters;
typedef ReferenceCountedObjectPtr<Lin09Parameters> Lin09ParametersPtr;  

extern ClassPtr lin09ParametersClass;

class Lin09Parameters : public Object
{
public:
  // global
  bool useProteinLength;
  bool useAminoAcidGlobalHistogram;
  bool usePSSMGlobalHistogram;
  bool useNumCysteins;
  // residue
  size_t aminoAcidWindowSize;
  size_t pssmWindowSize;
  size_t separationProfilSize;
  size_t aminoAcidLocalHistogramSize;
  size_t pssmLocalHistogramSize;
  // pair
  bool useSymmetricFeature;
  bool useAminoAcidIntervalHistogram;
  bool usePSSMIntervalHistogram;
  bool useAADistance;
  bool useCysteinDistance;
  
  bool usePositionDifference;
  bool useIndexDifference;

  bool hdSeparationProfil;

  Lin09Parameters()
  : // global
    useProteinLength(false)
  , useAminoAcidGlobalHistogram(false), usePSSMGlobalHistogram(false)
  , useNumCysteins(false)
    // residue
  , aminoAcidWindowSize(0), pssmWindowSize(0)
  , separationProfilSize(0), aminoAcidLocalHistogramSize(0), pssmLocalHistogramSize(0)
    // pair
  , useSymmetricFeature(false)
  , useAminoAcidIntervalHistogram(false), usePSSMIntervalHistogram(false)
  , useAADistance(false), useCysteinDistance(false)
  , usePositionDifference(false), useIndexDifference(false)

  , hdSeparationProfil(false)
  {}

  static std::vector<StreamPtr> createStreams()
  {
    ClassPtr thisType = lin09ParametersClass;
    const size_t n = thisType->getNumMemberVariables();
    std::vector<StreamPtr> res(n);
    
    for (size_t i = 0; i < n; ++i)
    {
      const TypePtr varType = thisType->getMemberVariableType(i);
      const String varName = thisType->getMemberVariableName(i);
      if (varType->inheritsFrom(booleanType))
        res[i] = booleanStream(true);
      else if (varName == T("aminoAcidWindowSize")
               || varName == T("pssmWindowSize"))
      {
        std::vector<int> values;
        values.push_back(5);
        values.push_back(15);
        values.push_back(25);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("aminoAcidLocalHistogramSize")
               || varName == T("pssmLocalHistogramSize"))
      {
        std::vector<int> values;
        values.push_back(25);
        values.push_back(50);
        values.push_back(100);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else if (varName == T("separationProfilSize"))
      {
        std::vector<int> values;
        values.push_back(3);
        values.push_back(5);
        values.push_back(9);
        values.push_back(11);
        res[i] = integerStream(positiveIntegerType, values);
      }
      else
        jassertfalse;
    }
    return res;
  }

  static Lin09ParametersPtr createInitialObject()
  {
    Lin09ParametersPtr res = new Lin09Parameters();
    return res;
  }
};

class Lin09PredictorParameters : public ProteinPredictorParameters
{
public:
  Lin09PredictorParameters(Lin09ParametersPtr fp = new Lin09Parameters())
    : fp(fp)
    , C(7.4), kernelGamma(-4.6)
    , useLibSVM(true), useLibLinear(false), useLaRank(false), useKNN(false), useExtraTrees(false)
    , learningRate(100.0), numIterations(150), numNeighbors(1)
    , useAddBias(true)
  {setThisClass(lin09PredictorParametersClass);}
  
  SamplerPtr createSampler() const
  {
    std::vector<SamplerPtr> res(lin09PredictorParametersClass->getNumMemberVariables());
    
    for (size_t i = 0; i < res.size(); ++i)
    {
      const String varName = lin09PredictorParametersClass->getMemberVariableName(i);
      if (varName == T("fp"))
      {
        Lin09ParametersPtr param = new Lin09Parameters();
        param->separationProfilSize = 9;
        res[i] = constantSampler(param);
      }
      else if (varName == T("C"))
        res[i] = gaussianSampler(C, 10);
      else if (varName == T("kernelGamma"))
        res[i] = gaussianSampler(kernelGamma, 10);
      else
        res[i] = constantSampler(getVariable(i));
    }
    
    return objectCompositeSampler(lin09PredictorParametersClass, res);
  }
  
  /*
  ************************ Protein Perception ************************
  */
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();

      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(new ProteinLengthFunction(), protein);
      builder.addFunction(new NumCysteinsFunction(), protein);
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));  
      // AA
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, aaResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      // PSSM
      primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, pssmResidueFeatures)), length, protein);
      primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);

    size_t perception = builder.finishSelectionWithFunction(new CreateLin09ProteinPerception());
    size_t globalFeatures = builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, globalFeatures), perception, T("globalFeatures"));

    builder.addFunction(new UpdateGlobalFeatureProteinPerception(), perception, globalFeatures, T("proteinPerception"));
  }

  /*
  ************************ Property Perception ************************
  */
  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  void globalFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType), T("proteinPerception"));
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccumulator"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccumulator"));
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));

    /* Data */
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception);
    size_t numCys = builder.addFunction(getVariableFunction(T("numCysteins")), proteinPerception);
    
    builder.startSelection();
      // protein length
      if (fp->useProteinLength)
        builder.addFunction(integerFeatureGenerator(), length, T("length"));

      // global composition
      if (fp->useAminoAcidGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator, T("h(AA)"));

      if (fp->usePSSMGlobalHistogram)
        builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));

      // number of cysteins
      if (fp->useNumCysteins)
      {
        builder.addFunction(integerFeatureGenerator(), numCys, T("#Cys"));
        builder.addFunction(new IsNumCysteinPair(), protein, T("(#Cys+1) % 2"));
      }
      
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
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

  /*
  ************************ Residue Perception ************************
  */
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));

    /* Data */
    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t cysteinIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, position);

    size_t aaResidueFeatures = builder.addFunction(getVariableFunction(T("aaResidueFeatures")), proteinPerception);
    size_t pssmResidueFeatures = builder.addFunction(getVariableFunction(T("pssmResidueFeatures")), proteinPerception);
    
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccumulator"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccumulator"));

    size_t cysteinSeparationProfil = builder.addFunction(new CreateCysteinSeparationProfil(true), protein, position, T("SepProfil"));
    if (fp->hdSeparationProfil)
      cysteinSeparationProfil = builder.addFunction(mapContainerFunction(hardDiscretizedNumberFeatureGenerator(0, 300, 30, true)), cysteinSeparationProfil);
    else
      cysteinSeparationProfil = builder.addFunction(mapContainerFunction(doubleFeatureGenerator()), cysteinSeparationProfil);

    /* Output */
    builder.startSelection();
      if (fp->aminoAcidWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->aminoAcidWindowSize), aaResidueFeatures, position, T("AminoAcid"));
      if (fp->pssmWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->pssmWindowSize), pssmResidueFeatures, position, T("PSSM"));

      if (fp->aminoAcidLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->aminoAcidLocalHistogramSize), aaAccumulator, position, T("aaLocalHisto[") + String((int)fp->aminoAcidLocalHistogramSize) + T("]"));
      if (fp->pssmLocalHistogramSize)
        builder.addFunction(accumulatorLocalMeanFunction(fp->pssmLocalHistogramSize), pssmAccumulator, position, T("pssmLocalHisto[") + String((int)fp->pssmLocalHistogramSize) + T("]"));
      if (fp->separationProfilSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(fp->separationProfilSize), cysteinSeparationProfil, cysteinIndex, T("SepProfil"));

      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0)); // AntiCrash
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /*
  ************************ Cystein Residue Perception ************************
  */
  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
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
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));
    
    builder.startSelection();
    
    builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
    builder.addInSelection(proteinPerception);
    
    builder.finishSelectionWithFunction(new CreateDisulfideMatrixFunction(
            lbcppMemberCompositeFunction(Lin09PredictorParameters, cysteinResiduePairVectorFeatures))
                                        , T("cysteinResiduePairFeatures"));
  }

  virtual void cysteinSymmetricResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));
    
    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(Lin09PredictorParameters, cysteinResiduePairVectorFeatures))
                                        , T("cysteinResiduePairFeatures"));
  }
  
  void cysteinResiduePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(lin09ProteinPerceptionClass(enumValueType, enumValueType, enumValueType));

    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);
    
    size_t aaAccumulator = builder.addFunction(getVariableFunction(T("aaAccumulator")), proteinPerception, T("aaAccumulator"));
    size_t pssmAccumulator = builder.addFunction(getVariableFunction(T("pssmAccumulator")), proteinPerception, T("pssmAccumulator"));
    
    size_t rf1 = builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, residueFeatures), firstPosition, proteinPerception, T("rf[first]"));
    size_t rf2 = builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, residueFeatures), secondPosition, proteinPerception, T("rf[second]"));
    
    size_t cysDist = builder.addFunction(new SubtractFunction(), secondIndex, firstIndex, T("|CYS2-CYS1|"));
    size_t aaDist = builder.addFunction(new SubtractFunction(), secondPosition, firstPosition, T("|AA2-AA1|"));

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

      if (fp->useCysteinDistance)
        builder.addFunction(doubleFeatureGenerator(), cysDist, T("|CYS2-CYS1|"));

      if (fp->usePositionDifference)
      builder.addFunction(new NormalizedCysteinPositionDifference(), protein, firstPosition, secondPosition, T("NCPD"));
      if (fp->useIndexDifference)
        builder.addFunction(new NormalizedCysteinIndexDifference(), protein, firstIndex, secondIndex, T("NCID"));

      if (fp->useAminoAcidIntervalHistogram)
        builder.addFunction(accumulatorWindowMeanFunction(), aaAccumulator, firstPosition, secondPosition, T("h(AA1,AA2)"));
      if (fp->usePSSMIntervalHistogram)
        builder.addFunction(accumulatorWindowMeanFunction(), pssmAccumulator, firstPosition, secondPosition, T("h(PSSM1,PSSM2)"));

      if (fp->useAADistance)
        builder.addFunction(doubleFeatureGenerator(), aaDist, T("|AA2-AA1|"));
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  void cysteinResiduePairFeatures(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  /*
   ************************ Multi Task Features ************************
   */

  // Learning Machine
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    if ((target == dsbTarget || target == fdsbTarget) && useAddBias)
      return new ConnectivityPatternClassifier(learningMachine(target), target == dsbTarget);
    return ProteinPredictorParameters::createTargetPredictor(target);
  }

  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    switch (target)
    {
    case fdsbTarget:
    case dsbTarget:
      {
        if (useLibSVM)
          return libSVMBinaryClassifier(pow(2.0, C), rbfKernel, 0, pow(2.0, kernelGamma), 0.0);
        if (useLibLinear)
          return libLinearBinaryClassifier(pow(2.0, C), l2RegularizedL2LossDual);
        if (useLaRank)
          return laRankBinaryClassifier(pow(2.0, C), laRankRBFKernel, 0, pow(2.0, kernelGamma), 0.0);
        if (useKNN)
          return binaryNearestNeighbor(numNeighbors, false);
        if (useExtraTrees)
          return binaryClassificationExtraTree(defaultExecutionContext(), 1000, 10, 0);

        FunctionPtr classifier = linearBinaryClassifier(new StochasticGDParameters(constantIterationFunction(learningRate), StoppingCriterionPtr(), numIterations));
        classifier->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
        return classifier;
      }
    default:
      {
        jassertfalse;
        return FunctionPtr();
      }
    };
  }

public:
  friend class Lin09PredictorParametersClass;
  
  Lin09ParametersPtr fp;

  double C;
  double kernelGamma;
  bool useLibSVM;
  bool useLibLinear;
  bool useLaRank;
  bool useKNN;
  bool useExtraTrees;
  double learningRate;
  size_t numIterations;
  size_t numNeighbors;
  bool useAddBias;
  String inputDirectory;
};

typedef ReferenceCountedObjectPtr<Lin09PredictorParameters> Lin09PredictorParametersPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_
