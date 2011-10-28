/*-----------------------------------------.---------------------------------.
| Filename: ESANN11.h                      | Re-implementation of tools      |
| Author  : Julien Becker                  | used in our ESANN paper (2011)  |
| Started : 19/10/2011 09:05               | F.Maes, J.Becker & L.Wehenkel   |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

namespace lbcpp
{

extern ClassPtr esann11ProteinPerceptionClass(TypePtr aaType = anyType, TypePtr pssmType = anyType,
                                              TypePtr ss3Type = anyType, TypePtr ss8Type = anyType,
                                              TypePtr saType = anyType, TypePtr drType = anyType,
                                              TypePtr stalType = anyType);

class ESANN11ProteinPerception : public ProteinPrimaryPerception
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

  ESANN11ProteinPerception(TypePtr aaType, TypePtr pssmType, TypePtr ss3Type, TypePtr ss8Type, TypePtr saType, TypePtr drType, TypePtr stalType)
    : ProteinPrimaryPerception(esann11ProteinPerceptionClass(aaType, pssmType, ss3Type, ss8Type, saType, drType, stalType)) {}
  ESANN11ProteinPerception(TypePtr thisClass)
    : ProteinPrimaryPerception(thisClass) {}

protected:
  friend class ESANN11ProteinPerceptionClass;

  ESANN11ProteinPerception() {}
};

typedef ReferenceCountedObjectPtr<ESANN11ProteinPerception> ESANN11ProteinPerceptionPtr;

class CreateESANN11ProteinPerception : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return esann11ProteinPerceptionClass()->getNumMemberVariables();}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return esann11ProteinPerceptionClass()->getMemberVariableType(index);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    size_t index = esann11ProteinPerceptionClass()->getBaseType()->getNumMemberVariables();
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
    return esann11ProteinPerceptionClass(aaType, pssmType, ss3Type, ss8Type, saType, drType, stalType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ESANN11ProteinPerceptionPtr res = new ESANN11ProteinPerception(getOutputType());
    const size_t n = getNumInputs();
    for (size_t i = 0; i < n; ++i)
      res->setVariable(i, inputs[i]);
    return res;
  }
};

class ESANN11PredictorParameters : public ProteinPredictorParameters
{
public:
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

  virtual void residuePerception(CompositeFunctionBuilder& builder) const
  {
    enum {windowsSize = 15};
    enum {histogramsSize = 15};
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(largeProteinPerceptionClass());
    /* Data */
    size_t length = builder.addFunction(getVariableFunction(T("length")), proteinPerception, T("length"));

    size_t aaResidueFeatures   = builder.addFunction(getVariableFunction(T("aaResidueFeatures")),   proteinPerception, T("pssmRF"));
    size_t aaAccumulator       = builder.addFunction(getVariableFunction(T("aaAccumulator")),       proteinPerception, T("aaAccu"));
    size_t pssmResidueFeatures = builder.addFunction(getVariableFunction(T("pssmResidueFeatures")), proteinPerception, T("pssmRF"));
    size_t pssmAccumulator     = builder.addFunction(getVariableFunction(T("pssmAccumulator")),     proteinPerception, T("pssmAccu"));
    size_t ss3ResidueFeatures  = builder.addFunction(getVariableFunction(T("ss3ResidueFeatures")),  proteinPerception, T("ss3RF"));
    size_t ss3Accumulator      = builder.addFunction(getVariableFunction(T("ss3Accumulator")),      proteinPerception, T("ss3Accu"));
    size_t ss8ResidueFeatures  = builder.addFunction(getVariableFunction(T("ss8ResidueFeatures")),  proteinPerception, T("ss8RF"));
    size_t ss8Accumulator      = builder.addFunction(getVariableFunction(T("ss8Accumulator")),      proteinPerception, T("ss8Accu"));
    size_t saResidueFeatures   = builder.addFunction(getVariableFunction(T("saResidueFeatures")),   proteinPerception, T("saRF"));
    size_t saAccumulator       = builder.addFunction(getVariableFunction(T("saAccumulator")),       proteinPerception, T("saAccu"));
    size_t drResidueFeatures   = builder.addFunction(getVariableFunction(T("drResidueFeatures")),   proteinPerception, T("drRF"));
    size_t drAccumulator       = builder.addFunction(getVariableFunction(T("drAccumulator")),       proteinPerception, T("drAccu"));
    size_t stalResidueFeatures = builder.addFunction(getVariableFunction(T("stalResidueFeatures")), proteinPerception, T("stalRF"));
    size_t stalAccumulator     = builder.addFunction(getVariableFunction(T("stalAccumulator")),     proteinPerception, T("stalAccu"));
    /* Precompute */
    size_t relativePosition = builder.addFunction(new RelativeValueFeatureGenerator(1), position, length, T("Pos/Len"));
    /* Output */
    builder.startSelection();
      /*** Global Features ***/
      builder.addFunction(defaultPositiveIntegerFeatureGenerator(), length, T("length"));

    /*
     extern PerceptionPtr defaultPositiveIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
     softDiscretizedLogNumberFeatures(positiveIntegerType, 0.0, maxPowerOfTen, numIntervals, true)
     
     extern PerceptionPtr defaultIntegerFeatures(size_t numIntervals = 20, double maxPowerOfTen = 10.0);
     signedNumberFeatures(softDiscretizedLogNumberFeatures(integerType, 0.0, maxPowerOfTen, numIntervals, true))
     
     extern PerceptionPtr defaultDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
     signedNumberFeatures(defaultPositiveDoubleFeatures(numIntervals, minPowerOfTen, maxPowerOfTen))
     
     extern PerceptionPtr defaultPositiveDoubleFeatures(size_t numIntervals = 20, double minPowerOfTen = -10.0, double maxPowerOfTen = 10.0);
     softDiscretizedLogNumberFeatures(doubleType, minPowerOfTen, maxPowerOfTen, numIntervals, true)
     
     extern PerceptionPtr defaultProbabilityFeatures(size_t numIntervals = 5);
     softDiscretizedNumberFeatures(probabilityType, 0.0, 1.0, numIntervals, false, false)

     rewriter->addRule(booleanType, booleanFeatures());
     rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
     rewriter->addRule(probabilityType, defaultProbabilityFeatures());
     rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
     rewriter->addRule(integerType, defaultIntegerFeatures());
     */
    
      // global histograms
      builder.addFunction(accumulatorGlobalMeanFunction(), aaAccumulator,   T("h(AA)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), pssmAccumulator, T("h(PSSM)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), ss3Accumulator,  T("h(SS3)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), ss8Accumulator,  T("h(SS8)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), saAccumulator,   T("h(SA)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), drAccumulator,   T("h(DR)"));
      builder.addFunction(accumulatorGlobalMeanFunction(), stalAccumulator, T("h(StAl)"));

      /*** Residue Features ***/
      //builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator()), relativePosition);
      builder.addInSelection(relativePosition);
      // TODO: use distance from begin
      // TODO: use distance to end

      // window sizes
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), aaResidueFeatures,   position, T("w(AA,")   + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), pssmResidueFeatures, position, T("w(PSSM,") + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), ss3ResidueFeatures,  position, T("w(SS3,")  + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), ss8ResidueFeatures,  position, T("w(SS8,")  + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), saResidueFeatures,   position, T("w(SA,")   + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), drResidueFeatures,   position, T("w(DR,")   + String((int)windowsSize) + (")"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(windowsSize), stalResidueFeatures, position, T("w(StAl,") + String((int)windowsSize) + (")"));

      // local histogram
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), aaAccumulator,   position, T("h(AA,")   + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), pssmAccumulator, position, T("h(PSSM,") + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), ss3Accumulator,  position, T("h(SS3,")  + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), ss8Accumulator,  position, T("h(SS8,")  + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), saAccumulator,   position, T("h(SA,")   + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), drAccumulator,   position, T("h(DR,")   + String((int)histogramsSize) + T(")"));
      builder.addFunction(accumulatorLocalMeanFunction(histogramsSize), stalAccumulator, position, T("h(StAl,") + String((int)histogramsSize) + T(")"));
    
      // bias (and anti-crash)
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));
    
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
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
    
    size_t elements = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("pssm"));
    builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator()), elements);
  }
  
  void ss3ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t elements = builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position, T("ss3"));
    builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator()), elements);
  }
  
  void ss8ResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t elements = builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), protein, position, T("ss8"));
    builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator()), elements);
  }
  
  void saResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t sa = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), protein, position, T("sa"));
    builder.addFunction(defaultProbabilityFeatureGenerator(), sa, T("sa"));
  }
  
  void drResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), protein, position, T("dr"));
    builder.addFunction(defaultProbabilityFeatureGenerator(), dr, T("dr"));
  }
  
  void stalResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));
    
    size_t elements = builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), protein, position, T("stal"));
    builder.addFunction(mapContainerFunction(defaultProbabilityFeatureGenerator()), elements);
  }
/*
  virtual FunctionPtr createResidueVectorPerception() const
  {
    FunctionPtr res = ProteinPredictorParameters::createResidueVectorPerception();
    return composeFunction(res, mapContainerFunction(doubleVectorNormalizeFunction(true, false)));
  }
*/
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    double sgdRate = 1.f;
    size_t sgdIterations = 100;

    if (target == ss3Target)
    {
      sgdRate = 5.f;
    }
    else if (target == ss8Target)
    {
      sgdRate = 10.f;
    }
    else if (target == sa20Target)
    {
      sgdRate = 1.f;
    }
    else if (target == drTarget)
    {
      sgdRate = 1.f;
    }
    else if (target == stalTarget)
    {
      sgdRate = 1.f;
    }
    else
      jassertfalse;

    LearnerParametersPtr learningParameters = new StochasticGDParameters(constantIterationFunction(sgdRate), StoppingCriterionPtr(), sgdIterations);
    FunctionPtr res;

    switch (target)
    {
      case drTarget:
      {
        res = linearBinaryClassifier(learningParameters, true, binaryClassificationMCCScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore));
        res->setBatchLearner(balanceBinaryExamplesBatchLearner(res->getBatchLearner()));
        break;
      }
      case sa20Target:
      {
        res = linearBinaryClassifier(learningParameters, true, binaryClassificationAccuracyScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
        res->setBatchLearner(balanceBinaryExamplesBatchLearner(res->getBatchLearner()));
        break;
      }
      default:
      {
        res = linearLearningMachine(learningParameters);
        res->setEvaluator(defaultSupervisedEvaluator());
        break;
      }
    }
    
    return res;
  }
};

typedef ReferenceCountedObjectPtr<ESANN11PredictorParameters> ESANN11PredictorParametersPtr;

class ESANN11WorkUnit : public WorkUnit
{
public:
  ESANN11WorkUnit() : numStages(1) {}

  virtual Variable run(ExecutionContext& context)
  {
    ESANN11PredictorParametersPtr predictor = new ESANN11PredictorParameters();

    ContainerPtr trainingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("train/")), 0, T("Loading training proteins"));
    if (!trainingData || !trainingData->getNumElements())
    {
      context.errorCallback(T("No training proteins !"));
      return 101.f;
    }

    ProteinSequentialPredictorPtr stack = new ProteinSequentialPredictor();
    for (size_t i = 0; i < numStages; ++i)
    {
      ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
      //iteration->addTarget(ss3Target);
      //iteration->addTarget(ss8Target);
      //iteration->addTarget(sa20Target);
      iteration->addTarget(drTarget);
      //iteration->addTarget(stalTarget);

      iteration->setEvaluator(new ProteinEvaluator());
      stack->addPredictor(iteration);
    }

    if (!stack->train(context, trainingData, ContainerPtr(), T("Training")))
      return 102.f;

    ContainerPtr testingData = Protein::loadProteinsFromDirectoryPair(context, File(), context.getFile(proteinsPath).getChildFile(T("test/")), 0, T("Loading training proteins"));
    if (!testingData || !testingData->getNumElements())
    {
      context.warningCallback(T("No testing proteins ! Training score is returned !"));
      return 103.f;
    }

    ProteinEvaluatorPtr trainEvaluator = new ProteinEvaluator();
    stack->evaluate(context, trainingData, trainEvaluator, T("Evaluate on train proteins"));

    ProteinEvaluatorPtr testEvaluator = new ProteinEvaluator();
    CompositeScoreObjectPtr testScores = stack->evaluate(context, testingData, testEvaluator, T("Evaluate on test proteins"));

    if (predictionPath != String::empty)
    {
      savePredictionsToDirectory(context, stack, trainingData, File(predictionPath).getChildFile(T("train")));
      savePredictionsToDirectory(context, stack, testingData, File(predictionPath).getChildFile(T("test")));
    }

    return true;
  }

protected:
  friend class ESANN11WorkUnitClass;

  String proteinsPath;
  String predictionPath;
  size_t numStages;

  bool savePredictionsToDirectory(ExecutionContext& context, FunctionPtr predictor, ContainerPtr proteinPairs, const File& predictionDirectory) const
  {
    return predictor->evaluate(context, proteinPairs, saveToDirectoryEvaluator(predictionDirectory, T(".xml")),
                               T("Saving predictions to directory ") + predictionDirectory.getFileName());
  }
};

};
