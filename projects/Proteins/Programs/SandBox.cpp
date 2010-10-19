/*-----------------------------------------.---------------------------------.
| Filename: SandBox.cpp                    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h"
#include "Perception/ProteinPerception.h"
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();

///////////////////////////////////////////////

class ProxyOnlineLearner : public InferenceOnlineLearner
{
public:
  ProxyOnlineLearner(const std::vector<InferencePtr>& inferences)
    : inferences(inferences) {}
  ProxyOnlineLearner() {}

  virtual void startLearningCallback()
    {}

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
    {}

  virtual void episodeFinishedCallback(const InferencePtr& inference)
    {}

  virtual void passFinishedCallback(const InferencePtr& inference)
    {}

  virtual double getCurrentLossEstimate() const
  {
    double res = 0.0;
    for (size_t i = 0; i < inferences.size(); ++i)
    {
      const InferenceOnlineLearnerPtr& learner = inferences[i]->getOnlineLearner();
      jassert(learner);
      res += learner->getCurrentLossEstimate();
    }
    return res;
  }

  virtual bool wantsMoreIterations() const
  {
    for (size_t i = 0; i < inferences.size(); ++i)
    {
      const InferenceOnlineLearnerPtr& learner = inferences[i]->getOnlineLearner();
      jassert(learner);
      if (learner->wantsMoreIterations())
        return true;
    }
    return false;
  }

  virtual bool isLearningStopped() const
  {
    for (size_t i = 0; i < inferences.size(); ++i)
    {
      const InferenceOnlineLearnerPtr& learner = inferences[i]->getOnlineLearner();
      jassert(learner);
      if (!learner->isLearningStopped())
        return false;
    }
    return true;
  }

protected:
  friend class ComposeOnlineLearnerClass;

  std::vector<InferencePtr> inferences;
};

#include "../../../src/Perception/Modifier/SelectAndMakeProductsPerception.h"
#include "../../../src/Inference/NumericalInference/NumericalInference.h"
#include "../../../src/Inference/NumericalInference/LinearInference.h"
#include "../../../src/Inference/NumericalInference/MultiLinearInference.h"

class GraftingOnlineLearner : public ProxyOnlineLearner
{
public:
  GraftingOnlineLearner(PerceptionPtr perception, const std::vector<InferencePtr>& inferences)
    : ProxyOnlineLearner(inferences), learningStopped(false), perception(perception.checkCast<SelectAndMakeProductsPerception>(T("GraftingOnlineLearner")))
  {
    // create empty perception for candidates
    candidatesPerception = selectAndMakeProductsPerception(
                                  this->perception->getDecoratedPerception(),
                                  this->perception->getMultiplyFunction());

    // initialize scores mapping (inference -> first output index)
    size_t c = 0;
    for (size_t i = 0; i < inferences.size(); ++i)
    {
      size_t numOutputs = getNumOutputs(inferences[i]);
      jassert(numOutputs);
      if (numOutputs)
      {
        scoresMapping[inferences[i]] = c;
        c += numOutputs;
      }
    }
    jassert(c);

    // initialize candidate scores
    candidateScores.resize(c);
  }
  GraftingOnlineLearner() {}

  typedef std::vector<size_t> Conjunction;

  virtual void startLearningCallback()
  {
    learningStopped = false;
    generateCandidates();
    resetCandidateScores();
  }

  virtual void subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    jassert(supervision.exists());
    std::map<InferencePtr, size_t>::const_iterator it = scoresMapping.find(inference);
    if (it != scoresMapping.end())
      updateCandidateScores(inference, it->second, input, supervision, prediction);
  }

  virtual void stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
    {}

  virtual void episodeFinishedCallback(const InferencePtr& inference)
    {}

  virtual void passFinishedCallback(const InferencePtr& inference)
  {
    acceptCandidates();
    pruneParameters();
    generateCandidates();
    resetCandidateScores();
    MessageCallback::info(String::empty);
    MessageCallback::info(T("Grafting"), T("=== ") + String((int)perception->getNumConjunctions()) + T(" active, ")
      + String((int)candidatesPerception->getNumConjunctions()) + T(" candidates ==="));
  }

  virtual bool isLearningStopped() const
    {return ProxyOnlineLearner::isLearningStopped() && learningStopped;}

  virtual bool wantsMoreIterations() const
    {return ProxyOnlineLearner::wantsMoreIterations() || !learningStopped;}

protected:
  void generateCandidates()
  {
    std::set<Conjunction> conjunctions;
    makeSetFromVector(conjunctions, perception->getConjunctions());

    candidatesPerception->clearConjunctions();

    // add each inactive output variable
    size_t n = perception->getDecoratedPerception()->getNumOutputVariables();
    for (size_t i = 0; i < n; ++i)
    {
      Conjunction conjunction(1, i);
      if (conjunctions.find(conjunction) == conjunctions.end())
        candidatesPerception->addConjunction(conjunction);
    }
  }

  String conjunctionToString(const Conjunction& conjunction) const
  {
    String res;
    for (size_t i = 0; i < conjunction.size(); ++i)
    {
      res += perception->getDecoratedPerception()->getOutputVariableName(conjunction[i]);
      if (i < conjunction.size() - 1)
        res += T(" x ");
    }
    return res;
  }

  void acceptCandidates()
  {
    static const double regularizerWeight = 0.0001;

    // compute candidate scores
    std::vector<double> scores;
    Conjunction bestCandidate;
    double bestCandidateScore;
    computeCandidateScores(scores, bestCandidate, bestCandidateScore);

    // generate top-five
    std::multimap<double, String> sortedScores;
    for (size_t i = 0; i < scores.size(); ++i)
      sortedScores.insert(std::make_pair(scores[i], conjunctionToString(candidatesPerception->getConjunction(i))));
    size_t i = 0;
    for (std::multimap<double, String>::reverse_iterator it = sortedScores.rbegin(); i < 5 && it != sortedScores.rend(); ++i, ++it)
    {
      if (!it->first)
        break;
      MessageCallback::info(T("Grafting"), T("Top ") + String((int)i + 1) + T(": ") + it->second + T(" (") + String(it->first) + T(")"));
    }

    // select top candidate
    if (bestCandidateScore > regularizerWeight)
    {
      MessageCallback::info(T("Grafting"), T("Incorporating ") + conjunctionToString(bestCandidate));
      perception->addConjunction(bestCandidate);
    }
    else
    {
      MessageCallback::info(T("Grafting"), T("Finished!"));
      learningStopped = true;
    }
  }

  void pruneParameters()
  {
    // todo
  }

protected:
  friend class GraftingOnlineLearnerClass;

  bool learningStopped;

  SelectAndMakeProductsPerceptionPtr perception;
  SelectAndMakeProductsPerceptionPtr candidatesPerception;

  // Candidate Score:
  //   max_{outputs} Score(Candidate, Output)
  std::vector< std::pair<ObjectPtr, size_t> > candidateScores;
  std::map<InferencePtr, size_t> scoresMapping; // inference -> Index of first score vector

  size_t getNumOutputs(const InferencePtr& inference) const
  {
    TypePtr outputType = inference->getOutputType(inference->getInputType());
    if (outputType->inheritsFrom(doubleType))
      return outputType;
    else
      return outputType->getObjectNumVariables();
  }

  template<class Type>
  void makeSetFromVector(std::set<Type>& res, const std::vector<Type>& source)
  {
    for (size_t i = 0; i < source.size(); ++i)
      res.insert(source[i]);
  }

  void resetCandidateScores()
  {
    for (size_t i = 0; i < candidateScores.size(); ++i)
      candidateScores[i] = std::make_pair(ObjectPtr(), 0);
  }

  void computeCandidateScores(std::vector<double>& res, Conjunction& bestCandidate, double& bestCandidateScore) const
  {
    size_t numCandidates = candidatesPerception->getNumConjunctions();
    size_t numScores = candidateScores.size();

    res.clear();
    res.resize(numCandidates, 0.0);
    bestCandidateScore = 0.0;
    bestCandidate.clear();
    for (size_t i = 0; i < numCandidates; ++i)
    {
      double scoresMax = 0.0;
      for (size_t j = 0; j < numScores; ++j)
      {
        double score = getCandidateScore(i, j);
        if (score > scoresMax)
          scoresMax = score;
      }
      res[i] = scoresMax;
      if (scoresMax > bestCandidateScore)
      {
        bestCandidateScore = scoresMax;
        bestCandidate = candidatesPerception->getConjunction(i);
      }
    }
  }

  double getCandidateScore(size_t candidateNumber, size_t scoreNumber) const
  {
    const ObjectPtr& scores = candidateScores[scoreNumber].first;
    size_t examplesCount = candidateScores[scoreNumber].second;
    if (!scores)
      return 0.0;
    jassert(examplesCount);
    double invC = 1.0 / (double)examplesCount;

    jassert(scores->getNumVariables() == candidatesPerception->getNumConjunctions());
    Variable candidateScore = scores->getVariable(candidateNumber);
    if (candidateScore.isDouble())
      return fabs(candidateScore.getDouble()) * invC; // score of a single feature
    else
    {
      // score of a group of features
      jassert(candidateScore.isObject());
      return lbcpp::l1norm(candidateScore.getObject()) * invC;
    }
  }

  void updateCandidateScores(const NumericalInferencePtr& numericalInference, size_t firstScoreIndex, const Variable& input, const Variable& supervision, const Variable& prediction)
  {
    jassert(perception == numericalInference->getPerception());
    if (numericalInference.dynamicCast<LinearInference>())
    {
      const ScalarFunctionPtr& loss = supervision.getObjectAndCast<ScalarFunction>();
      double derivative = loss->computeDerivative(prediction.getDouble());
      lbcpp::addWeighted(candidateScores[firstScoreIndex].first, candidatesPerception, input, derivative);
      ++candidateScores[firstScoreIndex].second;
    }
    else if (numericalInference.dynamicCast<MultiLinearInference>())
    {
      const MultiClassLossFunctionPtr& loss = supervision.getObjectAndCast<MultiClassLossFunction>();
      ObjectPtr gradient;
      jassert(prediction.isObject());
      loss->compute(prediction.getObject(), NULL, &gradient, 1.0);

      size_t n = gradient->getNumVariables();
      for (size_t i = 0; i < n; ++i)
      {
        double derivative = gradient->getVariable(i).getDouble();
        lbcpp::addWeighted(candidateScores[firstScoreIndex + i].first, candidatesPerception, input, derivative);
        ++candidateScores[firstScoreIndex + i].second;
      }
    }
    else
      jassert(false); // Unsupported. There is a design issue here, the interface of NumericalInference should be extended
  }
};

InferenceOnlineLearnerPtr graftingOnlineLearner(PerceptionPtr perception, InferencePtr inference)
  {return new GraftingOnlineLearner(perception, std::vector<InferencePtr>(1, inference));}

///////////////////////////////////////////////

InferenceContextPtr createInferenceContext()
{
  return multiThreadedInferenceContext(new ThreadPool(7, false));
}

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    return res ? flattenPerception(res) : PerceptionPtr();
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
    {return binaryClassificationExtraTreeInference(targetName, perception, 2, 3);}

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
    {return classificationExtraTreeInference(targetName, perception, classes, 2, 3);}
};

class NumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual InferencePtr createTargetInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createTargetInference(targetName);
    //res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }
  
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
  {
    rewriter->addRule(booleanType, booleanFeatures());
    rewriter->addEnumValueFeaturesRule();
    rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
    rewriter->addRule(probabilityType, defaultProbabilityFeatures());
    rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
    rewriter->addRule(integerType, defaultIntegerFeatures());

    // all other features
    rewriter->addRule(doubleType, identityPerception());
  }

  std::vector<size_t> makeBinaryConjunction(size_t index1, size_t index2) const
    {std::vector<size_t> res(2); res[0] = index1; res[1] = index2; return res;}

  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    //return res;
    
    PerceptionPtr collapsedFeatures = collapsePerception(res);
/*
    std::vector< std::vector<size_t> > selectedConjunctions;
    for (size_t i = 0; i < collapsedFeatures->getNumOutputVariables(); ++i)
      selectedConjunctions.push_back(std::vector<size_t>(1, i));

    //    selectedConjunctions.push_back(makeBinaryConjunction(0, 1));
    //selectedConjunctions.push_back(makeBinaryConjunction(5, 10));
    //selectedConjunctions.push_back(makeBinaryConjunction(10, 15));
*/
    return selectAndMakeConjunctionFeatures(collapsedFeatures);
  }

public:
  virtual InferencePtr createContactMapInference(const String& targetName) const
  {
    InferencePtr res = ProteinInferenceFactory::createContactMapInference(targetName);
    res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
  {
    InferencePtr scoreInference = linearInference(targetName + T(" Classifier"), perception);
    scoreInference->setOnlineLearner(createOnlineLearner(targetName + T(" Learner")));
    // FIXME !
    //if (targetName.startsWith(T("contactMap")) || targetName == T("disorderRegions"))
    //  scoreInference = addBiasInference(targetName, scoreInference, 0.0);
    return binaryLinearSVMInference(scoreInference);
  }

  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
  {
    //StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(50);
    InferenceOnlineLearnerPtr multiLinearLearner = createOnlineLearner(targetName, 0.1);
    StaticDecoratorInferencePtr res = multiClassLinearSVMInference(perception, classes, multiLinearLearner, true, targetName);
//    res->setOnlineLearner(stoppingCriterionOnlineLearner(graftingOnlineLearner(perception, res->getSubInference()),
//      InferenceOnlineLearner::perPass, stoppingCriterion, true));
    res->setOnlineLearner(graftingOnlineLearner(perception, res->getSubInference()));
    return res;

   // return multiClassLinearSVMInference(perception, classes, createOnlineLearner(targetName, 0.5), false, targetName);

  /*
    InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    //res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;*/
  }

protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0) const
  {
      StoppingCriterionPtr stoppingCriterion;// = maxIterationsStoppingCriterion(5);/* logicalOr(
/*                                                     maxIterationsStoppingCriterion(5),
                                                     maxIterationsWithoutImprovementStoppingCriterion(1));*/

//    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(5);/*logicalOr(
      /*maxIterationsStoppingCriterion(100),
      maxIterationsWithoutImprovementStoppingCriterion(1));*/

    if (targetName.startsWith(T("contactMap")))
      return gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::perEpisode,                                                 // randomization
        InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
        InferenceOnlineLearner::perStepMiniBatch20, l2Regularizer(0.0),         // regularizer
        InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
    else
      return gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::perPass,                                                 // randomization
        InferenceOnlineLearner::perStep, constantIterationFunction(0.3)/* invLinearIterationFunction(initialLearningRate, 10000)*/, true, // learning steps
        InferenceOnlineLearner::never, l2Regularizer(0.0),         // regularizer
        InferenceOnlineLearner::perPass, stoppingCriterion, false);                     // stopping criterion
  }
};

/////////////////////////////////////////

class MyInferenceCallback : public InferenceCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData)
    : inference(inference), trainingData(trainingData), testingData(testingData) {}

  virtual void preInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      iterationNumber = 0;
    }

    if (input.size() == 2 && input[0].getType()->inheritsFrom(inferenceClass))
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      MessageCallback::info(T("=== Learning ") + input[0].getObject()->getName() + T(" with ") + String((int)input[1].size()) + T(" ") + inputTypeName + T("(s) ==="));
      //std::cout << "  learner: " << inferenceClassName << " static type: " << input[1].getTypeName() << std::endl
      //  << "  first example type: " << input[1][0].getTypeName() << std::endl << std::endl;
    }
  }

  virtual void postInferenceCallback(const InferenceContextPtr& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceName = stack->getCurrentInference()->getName();

    //if (stack->getCurrentInference()->getClassName() == T("RunSequentialInferenceStepOnExamples"))
    if (inferenceName == T("LearningPass"))
    {
      // end of learning iteration
      MessageCallback::info(String::empty);
      MessageCallback::info(T("====================================================="));
      MessageCallback::info(T("================ EVALUATION =========================  ") + String((Time::getMillisecondCounter() - startingTime) / 1000) + T(" s"));
      MessageCallback::info(T("====================================================="));

      //singleThreadedInferenceContext();
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      context->evaluate(inference, trainingData, evaluator);
      processResults(evaluator, true);

      evaluator = new ProteinEvaluator();
      context->evaluate(inference, testingData, evaluator);
      processResults(evaluator, false);

      MessageCallback::info(T("====================================================="));
    }
    else if (stack->getDepth() == 1)
    {
      MessageCallback::info(T("Bye: ") + String((Time::getMillisecondCounter() - startingTime) / 1000.0) + T(" seconds"));
    }
  }

  void processResults(ProteinEvaluatorPtr evaluator, bool isTrainingData)
    {std::cout << " == " << (isTrainingData ? "Training" : "Testing") << " Scores == " << std::endl << evaluator->toString() << std::endl;}

private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData;
  size_t iterationNumber;
  juce::uint32 startingTime;
};

/////////////////////////////////////////

VectorPtr loadProteins(const File& directory, ThreadPoolPtr pool)
{
#ifdef JUCE_DEBUG
  size_t maxCount =1;
#else
  size_t maxCount = 20;
#endif // JUCE_DEBUG
  return directoryFileStream(directory)->load(maxCount)->apply(loadFromFileFunction(proteinClass), pool)
    ->apply(proteinToInputOutputPairFunction(), false)->randomize();

//  return directoryPairFileStream(directory, directory)->load(maxCount)
//      ->apply(loadFromFilePairFunction(proteinClass, proteinClass), pool)->randomize();
}

void initializeLearnerByCloning(InferencePtr inference, InferencePtr inferenceToClone)
{
  inference->setBatchLearner(multiPassInferenceLearner(initializeByCloningInferenceLearner(inferenceToClone), inference->getBatchLearner()));
}

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  ThreadPoolPtr pool = new ThreadPool(7);

#ifdef JUCE_WIN32
  File workingDirectory(T("C:\\Projets\\lbcpp\\projects\\temp\\psipred"));
#else
  File workingDirectory(T("/data/PDB/PDB30Medium"));
#endif

  ContainerPtr trainProteins = loadProteins(workingDirectory.getChildFile(T("train")), pool);
  ContainerPtr testProteins = loadProteins(workingDirectory.getChildFile(T("test")), pool);
  std::cout << trainProteins->getNumElements() << " training proteins, " << testProteins->getNumElements() << " testing proteins" << std::endl;

  //ProteinInferenceFactoryPtr factory = new ExtraTreeProteinInferenceFactory();
  ProteinInferenceFactoryPtr factory = new NumericalProteinInferenceFactory();

  ProteinParallelInferencePtr inferencePass = new ProteinParallelInference();
  //inference->setProteinDebugDirectory(workingDirectory.getChildFile(T("proteins")));
  //inference->appendInference(factory->createInferenceStep(T("contactMap8Ca")));

  /*inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/
  //inferencePass->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  //inferencePass->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  //inferencePass->appendInference(factory->createInferenceStep(T("disorderRegions")));
  //inferencePass->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));

  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  //inference->appendInference(inferencePass);
  //inference->appendInference(inferencePass->cloneAndCast<Inference>());

  InferencePtr lastStep = factory->createInferenceStep(T("secondaryStructure"));
  inference->appendInference(lastStep);
  for (int i = 1; i < 5; ++i)
  {
    InferencePtr step = factory->createInferenceStep(T("secondaryStructure"));
    initializeLearnerByCloning(step, lastStep);
    inference->appendInference(step);
    lastStep = step;
  } 

  //inference->appendInference(factory->createInferenceStep(T("solventAccessibilityAt20p")));
  //inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));

  /*inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
  inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));*/

  //inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  
  /*inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
  inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));*/

  /*
  inference->appendInference(inferencePass);
  inference->appendInference(inferencePass->cloneAndCast<Inference>());*/

/*  std::cout << "Inference: " << std::endl;
  Variable(inference).printRecursively(std::cout, 2);*/


  InferenceContextPtr context = multiThreadedInferenceContext(pool);
  ProteinEvaluatorPtr evaluator = new ProteinEvaluator();

  /*
  ReferenceCountedObject::resetRefCountDebugInfo();
  context->crossValidate(inference, proteins, evaluator, 2);
  std::cout << evaluator->toString() << std::endl;
  ReferenceCountedObject::displayRefCountDebugInfo(std::cout);
  return 0;*/

  context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
  context->train(inference, trainProteins);

  {
    std::cout << "Check Evaluating..." << std::endl;
    EvaluatorPtr evaluator = new ProteinEvaluator();
    context->evaluate(inference, trainProteins, evaluator);
    std::cout << "============================" << std::endl << std::endl;
    std::cout << evaluator->toString() << std::endl << std::endl;
  }

  std::cout << "Saving inference ..." << std::flush;
  inference->saveToFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  std::cout << "Loading..." << std::flush;
  inference = Inference::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  for (size_t i = 7; i <= 7; i += 1)
  {
    std::cout << "Check Evaluating with " << (i ? i : 1) << " threads ..." << std::endl;
    EvaluatorPtr evaluator = new ProteinEvaluator();
    InferenceContextPtr context = multiThreadedInferenceContext(new ThreadPool(i ? i : 1, false));
    context->appendCallback(new MyInferenceCallback(inference, trainProteins, testProteins));
    context->evaluate(inference, trainProteins, evaluator);
  //  context->crossValidate(inference, proteins, evaluator, 2);
    std::cout << "============================" << std::endl << std::endl;
    std::cout << evaluator->toString() << std::endl << std::endl;
  }

#if 0
  std::cout << "Loading..." << std::flush;
  InferencePtr loadedInference = Inference::createFromFile(workingDirectory.getChildFile(T("NewStyleInference.xml")));
  std::cout << "ok." << std::endl;

  //printDifferencesRecursively(v, inference, T("inference"));

  std::cout << "Re-saving..." << std::flush;
  loadedInference->saveToFile(workingDirectory.getChildFile(T("NewStyleInference2.xml")));
  std::cout << "ok." << std::endl;

  std::cout << "Re-evaluating..." << std::endl;
  evaluator = new ProteinEvaluator();
  context->evaluate(loadedInference, trainProteins, evaluator);
  std::cout << "ok." << std::endl;
  std::cout << "============================" << std::endl << std::endl;
  std::cout << evaluator->toString() << std::endl << std::endl;
#endif // 0

  std::cout << "Tchao." << std::endl;
  return 0;
}
