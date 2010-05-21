
class DefaultInferenceLearnerCallback : public InferenceLearnerCallback
{
public:
  DefaultInferenceLearnerCallback() : regularizer(1.0), initialLearningRate(2.0), numberIterationLearningRate(150000), drProbability(0.2), drRegularizer(150.0), saRegularizer(150.0) {}
  
  virtual InferenceContextPtr createContext()
    {return singleThreadedInferenceContext();}
  
  virtual double getProbabilityToCreateAnExample(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision)
  {
    String inferenceStepName = stack->getInference(1)->getName();
    if (inferenceStepName.startsWith(T("DR")))
      return drProbability;
    return 1.0;
  }
  
  virtual ClassifierPtr createClassifier(InferenceStackPtr stack, FeatureDictionaryPtr labels)
  {
    static const bool useConstantLearningRate = false;
    
    IterationFunctionPtr learningRate = useConstantLearningRate ? invLinearIterationFunction(initialLearningRate, numberIterationLearningRate) : constantIterationFunction(1.0);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
    GradientBasedClassifierPtr classifier = maximumEntropyClassifier(learner, labels);
    classifier->setL2Regularizer(regularizer);

    String inferenceStepName = stack->getInference(1)->getName();
    if (inferenceStepName.startsWith(T("SA"))) {
      classifier->setL2Regularizer(saRegularizer);
      std::cout << "DefaultInferenceLearnerCallback::createClassifier - Regularizer: " << saRegularizer;
    }

    if (inferenceStepName.startsWith(T("DR")))
    {
      classifier->setL2Regularizer(drRegularizer);
      std::cout << "DefaultInferenceLearnerCallback::createClassifier - Regularizer: " << drRegularizer;
    }

    return classifier;
  }
  
  virtual RegressorPtr createRegressor(InferenceStackPtr stack)
  {
    static const bool useConstantLearningRate = true;
    
    IterationFunctionPtr learningRate = useConstantLearningRate ? invLinearIterationFunction(initialLearningRate, numberIterationLearningRate) : constantIterationFunction(0.5);
    GradientBasedLearnerPtr learner = stochasticDescentLearner(learningRate);  
    return generalizedLinearRegressor(learner, 0.0/*regularizer*/);
  }
  
  void setL2Regularizer(double regularizer)
    {this->regularizer = regularizer;}
  
  void setInitialLearningRate(double value)
    {this->initialLearningRate = value;}
  
  void setNumberInterationLearningRate(size_t value)
    {this->numberIterationLearningRate = value;}

  void setDRProbability(double value)
    {this->drProbability = value;}
  
  void setDRRegularizer(double value)
    {this->drRegularizer = value;}

  void setSARegularizer(double value)
    {this->saRegularizer = value;}
protected:
  double regularizer;
  double initialLearningRate;
  size_t numberIterationLearningRate;
  double drProbability;
  double drRegularizer;
  double saRegularizer;
};

typedef ReferenceCountedObjectPtr<DefaultInferenceLearnerCallback> DefaultInferenceLearnerCallbackPtr;

class EvaluationInferenceLearnerCallback : public InferenceLearnerCallback
{
public:
  EvaluationInferenceLearnerCallback(InferenceLearnerCallbackPtr factory = new DefaultInferenceLearnerCallback())
  : trainingEvaluation(new ProteinEvaluationCallback()), testingEvaluation(new ProteinEvaluationCallback())
  , factory(factory)
  , target(T("NO_TARGET"))
  , trainingScore(0.), testingScore(0.)
  {
    trainingEvaluation->startInferencesCallback(0); // initialize a zero score
    testingEvaluation->startInferencesCallback(0);
  }
  
  virtual void setTrainingEvaluation(ProteinEvaluationCallbackPtr trainingEvaluation)
  {
    jassert(trainingEvaluation);
    this->trainingEvaluation = trainingEvaluation;
    trainingScore = trainingEvaluation->getDefaultScoreForTarget(target);
  }
  
  virtual void setTestingEvaluation(ProteinEvaluationCallbackPtr testingEvaluation)
  {
    jassert(testingEvaluation);
    this->testingEvaluation = testingEvaluation;
    testingScore = testingEvaluation->getDefaultScoreForTarget(target);
  }
  
  virtual void setTarget(const String& targetName)
    {this->target = targetName;}
  
  virtual const String& getTarget() const
    {return target;}
  
  virtual InferenceContextPtr createContext()
    {jassert(factory); return factory->createContext();}
  
  virtual ClassifierPtr createClassifier(InferenceStackPtr stack, FeatureDictionaryPtr labels)
    {jassert(factory); return factory->createClassifier(stack, labels);}
  
  virtual RegressorPtr createRegressor(InferenceStackPtr stack)
    {jassert(factory); return factory->createRegressor(stack);}
  
  virtual double getProbabilityToCreateAnExample(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision)
    {jassert(factory); return factory->getProbabilityToCreateAnExample(stack, input, supervision);}
  
  void setFactory(InferenceLearnerCallbackPtr factory)
    {this->factory = factory;}
  
protected:  
  ProteinEvaluationCallbackPtr getTrainingEvaluation()
    {jassert(trainingEvaluation); return trainingEvaluation;}
  
  ProteinEvaluationCallbackPtr getTestingEvaluation()
    {jassert(testingEvaluation); return testingEvaluation;}
  
  double getTrainingScore()
    {return trainingScore;}
  
  double getTestingScore()
    {return testingScore;}
  
private:
  ProteinEvaluationCallbackPtr trainingEvaluation;
  ProteinEvaluationCallbackPtr testingEvaluation;
  
  InferenceLearnerCallbackPtr factory;

  String target;
  double trainingScore;
  double testingScore;
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
  
  virtual ClassifierPtr createClassifier(InferenceStackPtr stack, FeatureDictionaryPtr labels)
    {return factory->createClassifier(stack, labels);}
  
  virtual RegressorPtr createRegressor(InferenceStackPtr stack)
    {return factory->createRegressor(stack);}
  
  virtual double getProbabilityToCreateAnExample(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision)
    {return factory->getProbabilityToCreateAnExample(stack, input, supervision);}
  
  virtual void preLearningIterationCallback(size_t iterationNumber)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->preLearningIterationCallback(iterationNumber);
    }
  }
  
  // returns false if learning should stop (if at least one callback return false)
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
  {
    InferenceContextPtr validationContext = createContext();
    
    if (cache)
      validationContext->appendCallback(cacheInferenceCallback(cache, inference));
    
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
  
  virtual void preLearningStepCallback(InferencePtr step)
  {
    String target = step.dynamicCast<Protein1DInferenceStep>()->getTargetName();
    
    for (size_t i = 0; i < callbacks.size(); ++i)
    {
      callbacks[i]->setTarget(target);
      callbacks[i]->preLearningStepCallback(step);
    }
  }
  
  virtual void postLearningStepCallback(InferencePtr step)
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
    , startTimePass(juce::Time::getMillisecondCounter())
    , bestTrainingScore(-DBL_MAX), bestTestingScore(-DBL_MAX)
    {}
  
  ~GnuPlotInferenceLearnerCallback()
  {
    for (std::map< String, OutputStream* >::const_iterator iter = oTargetPass.begin(); iter != oTargetPass.end(); iter++)
      delete iter->second;
  }
  
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
  {
    if (getTrainingScore() > bestTrainingScore) {
      bestTrainingScore = getTrainingScore();
      bestTestingScore = getTestingScore();
    }
    
    return true;
  }
  
  virtual void preLearningStepCallback(InferencePtr step)
  {
    bestTrainingScore = -DBL_MAX;
    bestTestingScore = -DBL_MAX;
  }
  
  virtual void postLearningStepCallback(InferencePtr step)
  {
    if (!oTargetPass.count(getTarget())) {
      File dst = File::getCurrentWorkingDirectory().getChildFile(prefixFilename + T(".") + getTarget());
      dst.deleteFile();
      oTargetPass[getTarget()] = dst.createOutputStream();
      targetIteration[getTarget()] = 0;
    }
    
    jassert(oTargetPass[getTarget()]);
    
    *oTargetPass[getTarget()] << (int) targetIteration[getTarget()] << '\t'
    << bestTrainingScore << '\t'
    << bestTestingScore  << '\t'
    << (juce::Time::getMillisecondCounter() - startTimePass) / 1000. << '\n';
    oTargetPass[getTarget()]->flush();
    
    targetIteration[getTarget()]++;
  }
  
private:
  String prefixFilename;
  double startTimePass;
  
  std::map< String, OutputStream* > oTargetPass;
  std::map< String, size_t > targetIteration;
  
  double bestTrainingScore;
  double bestTestingScore;
};

class StoppingCriterionInferenceLearnerCallback : public EvaluationInferenceLearnerCallback
{
public:
  StoppingCriterionInferenceLearnerCallback(size_t maximumIteration)
  : stoppingCriterion(logicalOr(maxIterationsStoppingCriterion(25), maxIterationsWithoutImprovementStoppingCriterion(maximumIteration)))
  {}
  
  virtual void preLearningStepCallback(InferencePtr step)
  {stoppingCriterion->reset();}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
  {return !stoppingCriterion->shouldOptimizerStop(getTrainingScore());}
  
private:
  StoppingCriterionPtr stoppingCriterion;
};

#if 0 // Saving and restoring the best parameters is now done directly by the InferenceLearner
class BestStepKeeperInferenceLearnerCallback : public EvaluationInferenceLearnerCallback
{
public:
  BestStepKeeperInferenceLearnerCallback(ProteinInferencePtr proteinInference, String& prefixFilename)
  : proteinInference(proteinInference)
  , prefixFilename(prefixFilename)
  , bestScore(0.)
  {}
  
  virtual void preLearningStepCallback(InferencePtr step)
  {bestScore = 0.;}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
  {
    if (getTrainingScore() > bestScore)
    {
      File dst = File::getCurrentWorkingDirectory().getChildFile(prefixFilename);
      inference->saveToFile(dst);
      std::cout << "Save iteration: " << inference->getName()
      << " - Iteration: " << iterationNumber << std::endl;
      bestScore = getTrainingScore();
    }
    
    return true;
  }
  
  virtual void postLearningStepCallback(InferencePtr step)
  {
    int stepNumber = proteinInference->findStepNumber(step);
    jassert(stepNumber >= 0);

    File toLoad = File::getCurrentWorkingDirectory().getChildFile(prefixFilename + T("/decorated.inference/") + lbcpp::toString(stepNumber) + T("_") + step->getName() + T(".inference"));
    jassert(toLoad.exists());
    InferencePtr inference = Object::createFromFileAndCast<InferenceStep>(toLoad);
    jassert(inference);
    std::cout << "Loaded step: " << inference->getName() << std::endl;
    
    proteinInference->setSubStep((size_t)stepNumber, inference);
  }
  
private:
  ProteinInferencePtr proteinInference;
  
  String prefixFilename;
  size_t stepNumber;
  
  double bestScore;
};
#endif // 0

class StandardOutputInferenceLearnerCallback : public EvaluationInferenceLearnerCallback {
  virtual void preLearningIterationCallback(size_t iterationNumber)
    {std::cout << std::endl << " ================== ITERATION " << iterationNumber << " ================== " << std::endl;}
  
  // returns false if learning should stop
  virtual bool postLearningIterationCallback(InferencePtr inference, size_t iterationNumber)
  {
    std::cout << "Train evaluation: " << getTrainingEvaluation()->toString() << std::endl;
    std::cout << "Test evaluation: " << getTestingEvaluation()->toString() << std::endl;
    return true;
  }
  
  virtual void preLearningStepCallback(InferencePtr step)
  {
    String passName = step->getName();
    std::cout << std::endl << "=====================================================" << std::endl;
    std::cout << "======= LEARNING PASS " << passName << " ==========" << std::endl;
    std::cout << "=====================================================" << std::endl;
  }
};
