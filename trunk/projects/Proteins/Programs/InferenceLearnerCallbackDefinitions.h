
class DefaultInferenceLearnerCallback : public InferenceLearnerCallback
{
public:
  DefaultInferenceLearnerCallback() : regularizer(1.0), initialLearningRate(2.0), numberIterationLearningRate(150000) {}
  
  virtual InferenceContextPtr createContext()
    {return singleThreadedInferenceContext();}
  
  virtual ClassifierPtr createClassifier(ClassificationInferenceStepPtr step, FeatureDictionaryPtr labels)
  {
    static const bool useConstantLearningRate = false;
    
    IterationFunctionPtr learningRate = useConstantLearningRate ? invLinearIterationFunction(initialLearningRate, numberIterationLearningRate) : constantIterationFunction(1.0);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
    GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
    classifier->setL2Regularizer(regularizer);
    
    if (labels == SolventAccesibility2StateDictionary::getInstance()) {
      classifier->setL2Regularizer(150);
      std::cout << "DefaultInferenceLearnerCallback::createClassifier - Regularizer: 150";
    }

    return classifier;
  }
  
  virtual RegressorPtr createRegressor(RegressionInferenceStepPtr step)
  {
    static const bool useConstantLearningRate = true;
    
    IterationFunctionPtr learningRate = useConstantLearningRate ? invLinearIterationFunction(initialLearningRate, numberIterationLearningRate) : constantIterationFunction(0.5);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
    return leastSquaresLinearRegressor(learner, 0.0/*regularizer*/);
  }
  
  void setL2Regularizer(double regularizer)
    {this->regularizer = regularizer;}
  
  void setInitialLearningRate(double value)
    {this->initialLearningRate = value;}
  
  void setNumberInterationLearningRate(size_t value)
    {this->numberIterationLearningRate = value;}
  
protected:
  double regularizer;
  double initialLearningRate;
  size_t numberIterationLearningRate;
};

typedef ReferenceCountedObjectPtr<DefaultInferenceLearnerCallback> DefaultInferenceLearnerCallbackPtr;

class EvaluationInferenceLearnerCallback : public InferenceLearnerCallback
{
public:
  enum TargetType {UNKNOWN = -1, SS3, SS8, SA};
  enum {numberTarget = 3};
  
  EvaluationInferenceLearnerCallback(InferenceLearnerCallbackPtr factory = new DefaultInferenceLearnerCallback())
  : trainingEvaluation(new ProteinEvaluationCallback()), testingEvaluation(new ProteinEvaluationCallback())
  , factory(factory)
  , target(UNKNOWN), targetName(T("NO_TARGET"))
  , targetTrainingScore(0.), targetTestingScore(0.)
  {
    trainingEvaluation->startInferencesCallback(0); // initialize a zero score
    testingEvaluation->startInferencesCallback(0);
  }
  
  virtual void setTrainingEvaluation(ProteinEvaluationCallbackPtr trainingEvaluation)
  {
    jassert(trainingEvaluation);
    this->trainingEvaluation = trainingEvaluation;
    targetTrainingScore = trainingEvaluation->getDefaultScoreForTarget(targetName);
  }
  
  virtual void setTestingEvaluation(ProteinEvaluationCallbackPtr testingEvaluation)
  {
    jassert(testingEvaluation);
    this->testingEvaluation = testingEvaluation;
    targetTestingScore = testingEvaluation->getDefaultScoreForTarget(targetName);
  }
  
  virtual void setTargetName(String& targetName)
  {
    this->targetName = targetName;
    target = getTypeFromTargetName(targetName);
    jassert(target != UNKNOWN);
  }
  
  virtual InferenceContextPtr createContext()
    {jassert(factory); return factory->createContext();}
  
  virtual ClassifierPtr createClassifier(ClassificationInferenceStepPtr step, FeatureDictionaryPtr labels)
    {jassert(factory); return factory->createClassifier(step, labels);}
  
  virtual RegressorPtr createRegressor(RegressionInferenceStepPtr step)
    {jassert(factory); return factory->createRegressor(step);}
  
  void setFactory(InferenceLearnerCallbackPtr factory)
    {this->factory = factory;}
  
protected:  
  ProteinEvaluationCallbackPtr getTrainingEvaluation()
    {jassert(trainingEvaluation); return trainingEvaluation;}
  
  ProteinEvaluationCallbackPtr getTestingEvaluation()
    {jassert(testingEvaluation); return testingEvaluation;}
  
  TargetType getTypeFromTargetName(const String& targetName)
  {
    if (targetName == T("SecondaryStructureSequence"))
      return SS3;
    if (targetName == T("DSSPSecondaryStructureSequence"))
      return SS8;
    if (targetName == T("SolventAccessibilitySequence"))
      return SA;
    
    jassert(false);
    return UNKNOWN;
  }
  
  TargetType getTarget()
    {return target;}
  
  double getTargetTrainingScore()
    {return targetTrainingScore;}
  
  double getTargetTestingScore()
    {return targetTestingScore;}
  
private:
  ProteinEvaluationCallbackPtr trainingEvaluation;
  ProteinEvaluationCallbackPtr testingEvaluation;
  
  InferenceLearnerCallbackPtr factory;
  
  TargetType target;
  String targetName;
  double targetTrainingScore;
  double targetTestingScore;
};

typedef ReferenceCountedObjectPtr<EvaluationInferenceLearnerCallback> EvaluationInferenceLearnerCallbackPtr;

class MultiInferenceLearnerCallback : public InferenceLearnerCallback
{
public:
  MultiInferenceLearnerCallback(ObjectContainerPtr trainingData, ObjectContainerPtr testingData, InferenceLearnerCallbackPtr factory)
  : trainingData(trainingData), testingData(testingData), factory(factory)
  , cache(new InferenceResultCache()) {}
  
  virtual ~MultiInferenceLearnerCallback()
  {}
  
  virtual InferenceContextPtr createContext()
  {return factory->createContext();}
  
  virtual ClassifierPtr createClassifier(ClassificationInferenceStepPtr step, FeatureDictionaryPtr labels)
  {return factory->createClassifier(step, labels);}
  
  virtual RegressorPtr createRegressor(RegressionInferenceStepPtr step)
  {return factory->createRegressor(step);}
  
  virtual void preLearningIterationCallback(size_t iterationNumber)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->preLearningIterationCallback(iterationNumber);
    }
  }
  
  // returns false if learning should stop (if at least one callback return false)
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
  {
    InferenceContextPtr validationContext = createContext();
    
    if (cache)
      validationContext->appendCallback(new AutoSubStepsCacheInferenceCallback(cache, inference));
    
    ProteinEvaluationCallbackPtr trainingEvaluation = new ProteinEvaluationCallback();
    validationContext->appendCallback(trainingEvaluation);
    validationContext->runWithSupervisedExamples(inference, trainingData);
    validationContext->removeCallback(trainingEvaluation);
    
    ProteinEvaluationCallbackPtr testingEvaluation = new ProteinEvaluationCallback();
    validationContext->appendCallback(testingEvaluation);
    validationContext->runWithSupervisedExamples(inference, testingData);
    
    bool res = true;
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->setTrainingEvaluation(trainingEvaluation);
      callbacks[i]->setTestingEvaluation(testingEvaluation);
      if (!callbacks[i]->postLearningIterationCallback(inference, iterationNumber))
        res = false;
    }
    
    return res;
  }
  
  virtual void preLearningStepCallback(InferenceStepPtr step)
  {
    String target = step.dynamicCast<ProteinSequenceInferenceStep>()->getTargetName();
    
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->setTargetName(target);
      callbacks[i]->preLearningStepCallback(step);
    }
  }
  
  virtual void postLearningStepCallback(InferenceStepPtr step)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->postLearningStepCallback(step);
    }
  }
  
  void appendCallback(EvaluationInferenceLearnerCallbackPtr callback)
  {
    jassert(callback);
    callback->setFactory(factory);
    callbacks.push_back(callback);
  }
  
  void useCache(bool useCache)
  {
    if (!useCache)
      cache = ObjectPtr();
    else if (!cache)
      cache = new InferenceResultCache();
  }
  
private:
  std::vector< EvaluationInferenceLearnerCallbackPtr >callbacks;
  
  ObjectContainerPtr trainingData;
  ObjectContainerPtr testingData;
  
  InferenceLearnerCallbackPtr factory;
  InferenceResultCachePtr cache;
};

typedef ReferenceCountedObjectPtr<MultiInferenceLearnerCallback> MultiInferenceLearnerCallbackPtr;

class GnuPlotInferenceLearnerCallback : public EvaluationInferenceLearnerCallback
{
public:
  GnuPlotInferenceLearnerCallback(String& prefix)
  : prefixFilename(prefix)
  //    , startTimeIteration(0.0)
  , startTimePass(juce::Time::getMillisecondCounter())
  //    , oIteration(NULL)
  //    , iterationNumber(0), passNumber(0)
  , bestTrainingScore(0.), bestTestingScore(0.)
  {
    for (size_t i = 0; i < numberTarget; ++i) {
      oTargetPass[i] = NULL;
      targetIteration[i] = 0;
    }
  }
  
  ~GnuPlotInferenceLearnerCallback()
  {
    for (size_t i = 0; i < numberTarget; ++i)
      delete oTargetPass[i];
  }
  
  virtual void preLearningIterationCallback(size_t iterationNumber)
  {
    //  this->iterationNumber = iterationNumber;
  }
  
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
  {
    //    *oIteration << (int) iterationNumber << '\t'
    //               << getTrainingEvaluation()->getQ3Score() << '\t'
    //               << getTestingEvaluation()->getQ3Score()  << '\t'
    //               << getTrainingEvaluation()->getQ8Score() << '\t'
    //               << getTestingEvaluation()->getQ8Score()  << '\t'
    //               << getTrainingEvaluation()->getSA2Score() << '\t'
    //               << getTestingEvaluation()->getSA2Score()  << '\t'
    //               << (juce::Time::getMillisecondCounter() - startTimeIteration) / 1000. << '\t'
    //               << (int) passNumber << '\n';
    //    oIteration->flush();
    
    if (getTargetTrainingScore() > bestTrainingScore) {
      bestTrainingScore = getTargetTrainingScore();
      bestTestingScore = getTargetTestingScore();
    }
    
    return true;
  }
  
  virtual void preLearningStepCallback(InferenceStepPtr step)
  {
    //    startTimeIteration = juce::Time::getMillisecondCounter();
    //
    //    File dst = File::getCurrentWorkingDirectory().getChildFile(prefixFilename + T(".iter_") + lbcpp::toString(passNumber));
    //    dst.deleteFile();
    //    oIteration = dst.createOutputStream();
    bestTrainingScore = 0.;
    bestTestingScore = 0.;
  }
  
  virtual void postLearningStepCallback(InferenceStepPtr step)
  {
    //    jassert(oIteration);
    //    delete oIteration;
    jassert(getTarget() != UNKNOWN);
    
    if (!oTargetPass[getTarget()]) {
      static const juce::tchar* targetToString[] = {
        T("SS3"), T("SS8"), T("SA")
      };
      
      File dst = File::getCurrentWorkingDirectory().getChildFile(prefixFilename + T(".") + targetToString[getTarget()]);
      dst.deleteFile();
      oTargetPass[getTarget()] = dst.createOutputStream();
    }
    
    jassert(oTargetPass[getTarget()]);
    
    *oTargetPass[getTarget()] << (int) targetIteration[getTarget()] << '\t'
    << bestTrainingScore << '\t'
    << bestTestingScore  << '\t'
    << (juce::Time::getMillisecondCounter() - startTimePass) / 1000. << '\n';
    oTargetPass[getTarget()]->flush();
    
    targetIteration[getTarget()]++;
    //   ++passNumber;
  }
  
private:
  String prefixFilename;
  
  //  double startTimeIteration;
  double startTimePass;
  
  OutputStream* oTargetPass[numberTarget];
  //  OutputStream* oIteration;
  
  size_t targetIteration[numberTarget];
  //  size_t iterationNumber;
  //  size_t passNumber;
  
  double bestTrainingScore;
  double bestTestingScore;
};

class StoppingCriterionInferenceLearnerCallback : public EvaluationInferenceLearnerCallback
{
public:
  StoppingCriterionInferenceLearnerCallback(size_t maximumIteration)
  : stoppingCriterion(logicalOr(maxIterationsStoppingCriterion(25), maxIterationsWithoutImprovementStoppingCriterion(maximumIteration)))
  {}
  
  virtual void preLearningStepCallback(InferenceStepPtr step)
  {stoppingCriterion->reset();}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
  {return !stoppingCriterion->shouldOptimizerStop(getTargetTrainingScore());}
  
private:
  StoppingCriterionPtr stoppingCriterion;
};

class BestStepKeeperInferenceLearnerCallback : public EvaluationInferenceLearnerCallback
{
public:
  BestStepKeeperInferenceLearnerCallback(ProteinInferencePtr proteinInference, String& prefixFilename, size_t firstStepToLearn = 0)
  : proteinInference(proteinInference)
  , prefixFilename(prefixFilename), stepNumber(firstStepToLearn)
  , bestScore(0.)
  {}
  
  virtual void preLearningStepCallback(InferenceStepPtr step)
  {bestScore = 0.;}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
  {
    if (getTargetTrainingScore() > bestScore)
    {
      File dst = File::getCurrentWorkingDirectory().getChildFile(prefixFilename);
      inference->saveToFile(dst);
      std::cout << "Save iteration: " << inference->getName()
      << " - Iteration: " << iterationNumber << std::endl;
      bestScore = getTargetTrainingScore();
    }
    
    return true;
  }
  
  virtual void postLearningStepCallback(InferenceStepPtr step)
  {
    File toLoad = File::getCurrentWorkingDirectory().getChildFile(prefixFilename + T("/decorated.inference/") + lbcpp::toString(stepNumber) + T("_") + step->getName() + T(".inference"));
    jassert(toLoad.exists());
    InferenceStepPtr inference = Object::createFromFileAndCast<InferenceStep>(toLoad);
    jassert(inference);
    std::cout << "Loaded step: " << inference->getName() << std::endl;
    
    proteinInference->setSubStep(stepNumber, inference);
    ++stepNumber;
  }
  
private:
  ProteinInferencePtr proteinInference;
  
  String prefixFilename;
  size_t stepNumber;
  
  double bestScore;
};

class StandardOutputInferenceLearnerCallback : public EvaluationInferenceLearnerCallback {
  virtual void preLearningIterationCallback(size_t iterationNumber)
  {std::cout << std::endl << " ================== ITERATION " << iterationNumber << " ================== " << std::endl;}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferenceStepPtr inference, size_t iterationNumber)
  {
    std::cout << "Train evaluation: " << getTrainingEvaluation()->toString() << std::endl;
    std::cout << "Test evaluation: " << getTestingEvaluation()->toString() << std::endl;
    return true;
  }
  
  virtual void preLearningStepCallback(InferenceStepPtr step)
  {
    String passName = step->getName();
    std::cout << std::endl << "=====================================================" << std::endl;
    std::cout << "======= LEARNING PASS " << passName << " ==========" << std::endl;
    std::cout << "=====================================================" << std::endl;
  }
};
