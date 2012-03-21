/*-----------------------------------------.---------------------------------.
| Filename: LuapeClassificationSandBox.h   | Luape Classification Sand Box   |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
# define LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Execution/ExecutionTrace.h>
# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Data/Stream.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/Numerical.h> // for lbcpp::convertSupervisionVariableToEnumValue
# include "LuapeSoftStump.h"
# include "MetaMCSandBox.h"
# include "../Core/NestedMonteCarloOptimizer.h"
# include "../Core/SinglePlayerMCTSOptimizer.h"
# include "../../../src/Luape/Learner/OptimizerBasedSequentialWeakLearner.h"

namespace lbcpp
{

#ifdef JUCE_WIN32
# pragma warning(disable:4996)
#endif // JUCE_WIN32

class JDBDataParser : public TextParser
{
public:
  JDBDataParser(ExecutionContext& context, const File& file, DynamicClassPtr features, DefaultEnumerationPtr labels, bool sparseData = false)
    : TextParser(context, file), features(features), labels(labels), sparseData(sparseData), hasReadDatasetName(false), hasReadAttributes(false) {}
  JDBDataParser() {}

  virtual TypePtr getElementsType() const
    {return pairClass(features, labels);}

  virtual bool parseLine(char* line)
  {
    while (*line == ' ' || *line == '\t')
      ++line;
    if (*line == 0 || *line == ';')
      return true; // skip empty lines and comment lines
    if (!hasReadDatasetName)
    {
      hasReadDatasetName = true;
      return true;
    }
    if (!hasReadAttributes)
    {
      bool res = parseAttributes(line);
      hasReadAttributes = true;
      return res;
    }
    return parseExample(line);
  }

  bool parseAttributes(char* line)
  {
    std::vector< std::pair<String, int> > attributes; // kind: 0 = numerical, 1 = symbolic, 2 = skip

    bool isFirst = true;
    while (true)
    {
      char* name = strtok(isFirst ? line : NULL, " \t\n\r");
      char* kind = strtok(NULL, " \t\n\r");
      if (!name || !kind)
        break;
      isFirst = false;
      int k;
      if (!strcmp(kind, "numerical") || !strcmp(kind, "NUMERICAL"))
        k = 0;
      else if (!strcmp(kind, "symbolic") || !strcmp(kind, "SYMBOLIC"))
        k = 1;
      else if (!strcmp(kind, "name") || !strcmp(kind, "NAME"))
        k = 2;
      else
      {
        context.errorCallback(T("Could not recognize attribute type ") + String(kind).quoted());
        return false;
      }
      attributes.push_back(std::make_pair(name, k));
    }

    // only keep last symbolic attribute
    outputColumnIndex = (size_t)-1;
    for (int i = attributes.size() - 1; i >= 0; --i)
      if (attributes[i].second == 1)
      {
        if (outputColumnIndex == (size_t)-1)
          outputColumnIndex = (size_t)i;
        attributes[i].second = 2;
      }

    columnToVariable.resize(attributes.size(), -1);
    for (size_t i = 0; i < attributes.size(); ++i)
      if (attributes[i].second == 0)
      {
        String name = attributes[i].first;
        int index = features->findOrAddMemberVariable(context, name, doubleType);
        columnToVariable[i] = index;
      }
    return true;
  }

  bool parseExample(char* line)
  {
    ObjectPtr inputs = sparseData ? features->createSparseObject() : features->createDenseObject();
    Variable output;
    bool isFirst = true;
    for (size_t i = 0; true; ++i)
    {
      char* token = strtok(isFirst ? line : NULL, " \t\n");
      if (!token)
        break;
      isFirst = false;

      if (i == outputColumnIndex)
        output = Variable(labels->findOrAddElement(context, token), labels);
      else
      {
        int index = columnToVariable[i];
        if (index >= 0)
        {
          double value = strtod(token, NULL);
          inputs->setVariable((size_t)index, value);
        }
      }
    }
    setResult(new Pair(inputs, output));
    return true;
  }

protected:
  DynamicClassPtr features;
  DefaultEnumerationPtr labels;
  bool sparseData;

  bool hasReadDatasetName;
  bool hasReadAttributes;

  size_t outputColumnIndex;
  std::vector<int> columnToVariable;
};

class TestingSetParser : public TextParser
{
public:
  TestingSetParser(ExecutionContext& context, const File& file, ContainerPtr data)
    : TextParser(context, file), data(data) {}
  TestingSetParser() {}

  virtual TypePtr getElementsType() const
  {
    TypePtr exampleType = data->getElementsType();
    return pairClass(containerClass(exampleType), containerClass(exampleType));
  }

  virtual bool parseLine(char* line)
  {
    std::set<size_t> testingIndices;
    bool isFirst = true;
    for (size_t i = 0; true; ++i)
    {
      char* token = strtok(isFirst ? line : NULL, " \t\n");
      if (!token)
        break;
      isFirst = false;
      int index = strtol(token, NULL, 0);
      if (index < 1 || index > (int)data->getNumElements())
      {
        context.warningCallback(T("Invalid index ") + String(token) + T(" (num examples = ") + String((int)data->getNumElements()) + T(")"));
        //return false;
        continue;
      }
      size_t idx = (size_t)(index - 1);
      if (testingIndices.find(idx) != testingIndices.end())
        context.warningCallback(T("Redundant index ") + String(token));
      testingIndices.insert(idx);
    }

    size_t n = data->getNumElements();
    /*
    //for (std::set<size_t>::const_iterator it = testingIndices.begin(); it != testingIndices.end(); ++it)
    //  std::cout << *it << " ";
    std::cout << std::endl;
    for (size_t i = 0; i < n; ++i)
      if (testingIndices.find(i) == testingIndices.end())
        std::cout << i << " ";
    std::cout << std::endl;
    */

    TypePtr exampleType = data->getElementsType();
    VectorPtr learningData = vector(exampleType, 0);
    learningData->reserve(n - testingIndices.size());
    VectorPtr testingData = vector(exampleType, 0);
    testingData->reserve(testingIndices.size());

    for (size_t i = 0; i < n; ++i)
      if (testingIndices.find(i) == testingIndices.end())
        learningData->append(data->getElement(i));
      else
        testingData->append(data->getElement(i));

    setResult(new Pair(learningData, testingData));
    return true;
  }

protected:
  ContainerPtr data;
};

class RelevanceDrivenFeatureGenerationLearner : public IterativeLearner
{
public:
  RelevanceDrivenFeatureGenerationLearner(LuapeLearnerPtr baseLearner, size_t numIterations, size_t numActiveVariables, LuapeLearnerPtr targetLearner)
    : IterativeLearner(baseLearner->getObjective(), numIterations), baseLearner(baseLearner), numActiveVariables(numActiveVariables), targetLearner(targetLearner) {}
  RelevanceDrivenFeatureGenerationLearner() {}

  virtual LuapeNodePtr createInitialNode(ExecutionContext& context, const LuapeInferencePtr& problem)
    {return baseLearner->createInitialNode(context, problem);}

  virtual bool initialize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
    {return IterativeLearner::initialize(context, node, problem, examples);}

  virtual bool doLearningIteration(ExecutionContext& context, LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples, double& trainingScore, double& validationScore)
  {
    if (verbose)
    {
     /* for (size_t i = 0; i < problem->getNumActiveVariables(); ++i)
      {
        LuapeNodePtr activeVariable = problem->getActiveVariable(i);
        context.informationCallback(T("Active variable: ") + activeVariable->toShortString());
      }*/

      if (targetLearner)
      {
        problem->clearRootNode(context);
        targetLearner->learn(context, problem, examples);
        context.resultCallback(T("targetValidationScore"), problem->evaluatePredictions(context, problem->getValidationPredictions(), problem->getValidationSupervisions()));
      }
    }

    // learn
    problem->clearRootNode(context);
    node = baseLearner->learn(context, problem, examples);

    // evaluate
    if (verbose)
      evaluatePredictions(context, problem, trainingScore, validationScore);
  
    // retrieve node importances
    std::map<LuapeNodePtr, double> importances;
    LuapeUniverse::getImportances(problem->getRootNode(), importances);
   // if (verbose)
   //   LuapeUniverse::displayMostImportantNodes(context, importances);

    // sort nodes by importance
    std::multimap<double, LuapeNodePtr> nodeImportanceMap;
    for (std::map<LuapeNodePtr, double>::const_iterator it = importances.begin(); it != importances.end(); ++it)
      if (!it->first.isInstanceOf<LuapeInputNode>())
        nodeImportanceMap.insert(std::make_pair(it->second, it->first));
    
    // create new set of active variables
    problem->clearActiveVariables();
    for (std::multimap<double, LuapeNodePtr>::reverse_iterator it = nodeImportanceMap.rbegin(); it != nodeImportanceMap.rend(); ++it)
    {
      problem->addActiveVariable(it->second);
      if (problem->getNumActiveVariables() >= numActiveVariables)
        break;
    }
    LuapeUniverse::clearImportances(problem->getRootNode());
    return node;
  }
  
  virtual bool finalize(ExecutionContext& context, const LuapeNodePtr& node, const LuapeInferencePtr& problem, const IndexSetPtr& examples)
  {
    bestObjectiveValue = baseLearner->getBestObjectiveValue();
    return IterativeLearner::finalize(context, node, problem, examples);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    IterativeLearner::clone(context, target);
    if (baseLearner)
      target.staticCast<RelevanceDrivenFeatureGenerationLearner>()->baseLearner = baseLearner->cloneAndCast<LuapeLearner>();
    if (targetLearner)
      target.staticCast<RelevanceDrivenFeatureGenerationLearner>()->targetLearner = targetLearner->cloneAndCast<LuapeLearner>();
  }

protected:
  friend class RelevanceDrivenFeatureGenerationLearnerClass;

  LuapeLearnerPtr baseLearner;
  size_t numActiveVariables;
  LuapeLearnerPtr targetLearner;
};

//////////////////////////////////////////////////////

class LuapeClassificationWorkUnit : public WorkUnit
{
public:
  LuapeClassificationWorkUnit() : maxExamples(0), verbose(false), useExtendedOperators(false), useVectorOperators(false) {}

protected:
  friend class LuapeClassificationWorkUnitClass;

  size_t maxExamples;
  bool verbose;
  bool useExtendedOperators;
  bool useVectorOperators;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr labels) const
  { 
    static const bool sparseData = true;

    context.enterScope(T("Loading ") + file.getFileName());
    TextParserPtr parser;
    if (file.getFileExtension() == T(".jdb"))
      parser = new JDBDataParser(context, file, inputClass, labels, sparseData);
    else
      parser = classificationARFFDataParser(context, file, inputClass, labels, sparseData);
    ContainerPtr res = parser->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  static DenseDoubleVectorPtr convertExampleToVector(const ObjectPtr& example, const ClassPtr& dvClass)
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(dvClass);
    size_t n = res->getNumValues();
    jassert(n == example->getNumVariables());
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, example->getVariable(i).toDouble());
    return res;
  }

  static ObjectVectorPtr convertExamplesToVectors(const ContainerPtr& examples)
  {
    PairPtr p = examples->getElement(0).getObjectAndCast<Pair>();
    
    ClassPtr dvClass = denseDoubleVectorClass(variablesEnumerationEnumeration(p->getFirst().getType()), doubleType);
    TypePtr supType = p->getSecond().getType();
    ClassPtr exampleType = pairClass(dvClass, supType);

    size_t n = examples->getNumElements();

    ObjectVectorPtr res = new ObjectVector(exampleType, n);
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example =  examples->getElement(i).getObjectAndCast<Pair>();
      res->set(i, new Pair(exampleType, convertExampleToVector(example->getFirst().getObject(), dvClass), example->getSecond()));
    }
    return res;
  }

  bool makeSplits(ExecutionContext& context, const File& tsFile, DynamicClassPtr inputClass, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
    if (tsFile.existsAsFile())
    {
      TextParserPtr parser = new TestingSetParser(context, tsFile, data);
      ContainerPtr splits = parser->load(0);
      res.resize(splits->getNumElements());
      for (size_t i = 0; i < res.size(); ++i)
      {
        PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
        ContainerPtr train = convertExamplesToVectors(split->getFirst().getObjectAndCast<ObjectVector>());
        ContainerPtr test = convertExamplesToVectors(split->getSecond().getObjectAndCast<ObjectVector>());
        res[i] = std::make_pair(train, test);
      }
    }
    else
    { 
      const size_t numSplits = 20;
      const size_t numFolds = 10;

      res.resize(numSplits);
      for (size_t i = 0; i < numSplits; ++i)
      {
        ContainerPtr randomized = data->randomize();
        ContainerPtr training = convertExamplesToVectors(randomized->invFold(0, numFolds));
        ContainerPtr testing = convertExamplesToVectors(randomized->fold(0, numFolds));
        res[i] = std::make_pair(training, testing);
      }
    }
    return true;
  }

  LuapeInferencePtr createClassifier(TypePtr inputDoubleVectorType) const
  {
    LuapeInferencePtr res = new LuapeClassifier();

    if (useVectorOperators)
    {
      res->addInput(inputDoubleVectorType, "in");
      res->addFunction(getDoubleVectorElementLuapeFunction());
      res->addFunction(computeDoubleVectorStatisticsLuapeFunction());
      res->addFunction(getDoubleVectorExtremumsLuapeFunction());
      res->addFunction(getVariableLuapeFunction());
    }
    else
    {
      EnumerationPtr attributes = DoubleVector::getElementsEnumeration(inputDoubleVectorType);
      size_t n = attributes->getNumElements();
      for (size_t i = 0; i < n; ++i)
        res->addInput(doubleType, attributes->getElementName(i));
    }
    
    if (useExtendedOperators)
    {
      res->addFunction(logDoubleLuapeFunction());
      res->addFunction(sqrtDoubleLuapeFunction());
      res->addFunction(minDoubleLuapeFunction());
      res->addFunction(maxDoubleLuapeFunction());
    }

    res->addFunction(addDoubleLuapeFunction());
    res->addFunction(subDoubleLuapeFunction());
    res->addFunction(mulDoubleLuapeFunction());
    res->addFunction(divDoubleLuapeFunction());      
    return res;
  }

  double trainAndTestClassifier(ExecutionContext& context, const LuapeInferencePtr& inference, const LuapeLearnerPtr& learner,
     const ContainerPtr& trainingData, const ContainerPtr& testingData) const
  {
    if (!trainingData->getNumElements())
      return 0.0;
    inference->setSamples(context, trainingData.staticCast<ObjectVector>()->getObjects(), testingData.staticCast<ObjectVector>()->getObjects());
    //inference->getTrainingCache()->disableCaching();
    //inference->getValidationCache()->disableCaching();
    learner->learn(context, inference);
    return inference->evaluatePredictions(context, inference->getValidationPredictions(), inference->getValidationSupervisions());
  }
};

////////////////////////////////////////

class ECML12WorkUnit : public LuapeClassificationWorkUnit
{
public:
  ECML12WorkUnit() : method("ST"), ensembleSize(100), foldNumber(0), featureLength(0), featureBudget(10.0), searchAlgorithm(rollout()) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    File tsFile = dataFile.withFileExtension("txt");

    // load data
    DynamicClassPtr inputClass = new DynamicClass("inputs");
    DefaultEnumerationPtr labels = new DefaultEnumeration("labels");
    ContainerPtr data = loadData(context, dataFile, inputClass, labels);
    if (!data || !data->getNumElements())
      return false;

    // display data info
    size_t numVariables = inputClass->getNumMemberVariables();
    size_t numExamples = data->getNumElements();
    context.informationCallback(String((int)numExamples) + T(" examples, ") +
                                String((int)numVariables) + T(" variables, ") +
                                String((int)labels->getNumElements()) + T(" labels"));
//    context.informationCallback(String((int)trainingSize) + T(" training examples, ") + String((int)(numExamples - trainingSize)) + T(" testing examples"));

    // make splits
    std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;
    context.enterScope(T("Splits"));
    if (makeSplits(context, tsFile, inputClass, data, splits) && verbose)
    {
      for (size_t i = 0; i < splits.size(); ++i)
        context.informationCallback(T("Split ") + String((int)i) + T(": train size = ") + String((int)splits[i].first->getNumElements())
                              + T(", test size = ") + String((int)splits[i].second->getNumElements()));
    }
    context.leaveScope(splits.size());
    if (!splits.size())
      return false;
    if (foldNumber >= splits.size())
    {
      context.errorCallback(T("Invalid fold number"));
      return false;
    }
    ContainerPtr trainingData = splits[foldNumber].first;
    ContainerPtr testingData = splits[foldNumber].second;
    TypePtr inputDoubleVectorType = trainingData->getElementsType()->getTemplateArgument(0);

    // create classifier
    LuapeInferencePtr classifier = createClassifier(inputDoubleVectorType);
    if (!classifier->initialize(context, inputDoubleVectorType, labels))
      return false;


    // create learner
    LuapeLearnerPtr learner = createLearner(context, numVariables);
    if (!learner)
      return false;
    context.informationCallback(T("Learner: ") + learner->toShortString());

    // train and test
    context.enterScope("Learning");
    double res = trainAndTestClassifier(context, classifier, learner, trainingData, testingData);
    context.leaveScope(res);
    return res;
  }

protected:
  friend class ECML12WorkUnitClass;

  File dataFile;
  String method; // ST, RF, ETd, ETn, BS, BT2 .. BT100
  size_t ensembleSize;
  size_t foldNumber;
  size_t featureLength;
  double featureBudget; // percentage of "numVariables"
  MCAlgorithmPtr searchAlgorithm;

  LuapeLearnerPtr createLearner(ExecutionContext& context, size_t numVariables) const
  {
    bool useRandomSplits = method.startsWith(T("ET"));
    bool hasFeatureGeneration = (featureLength > 0);
    bool useRandomSubspaces = ((method.startsWith(T("ET")) && method != T("ETn")) || method == T("RF") || (featureBudget > 0 && featureBudget < 1));

    size_t budget = featureBudget ? (size_t)juce::jmax(1.0, numVariables * featureBudget) : (size_t)(0.5 + sqrt((double)numVariables));

    LuapeLearnerPtr conditionLearner;
    if (hasFeatureGeneration)
    {
      conditionLearner = optimizerBasedSequentialWeakLearner(new MCOptimizer(searchAlgorithm, budget), featureLength, useRandomSplits);
    }
    else
    {
      LuapeNodeBuilderPtr nodeBuilder = useRandomSubspaces ? (LuapeNodeBuilderPtr)randomSequentialNodeBuilder(budget, 2) : (LuapeNodeBuilderPtr)inputsNodeBuilder();
      conditionLearner = useRandomSplits ? (LuapeLearnerPtr)randomSplitWeakLearner(nodeBuilder) : (LuapeLearnerPtr)exactWeakLearner(nodeBuilder);
    }
    conditionLearner->setVerbose(verbose);

    LuapeLearnerPtr learner;
    if (method == T("ST"))
      learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    else if (method.startsWith(T("ET")))
      learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), ensembleSize);
    else if (method == T("RF"))
      learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), ensembleSize);
    else if (method == T("BS") || method.startsWith(T("BT")))
    {
      int treeDepth = (method == T("BS") ? 1 : method.substring(2).getIntValue());
      if (treeDepth <= 0)
      {
        context.errorCallback(T("Invalid tree size"));
        return LuapeLearnerPtr();
      }
      learner = discreteAdaBoostMHLearner(conditionLearner, ensembleSize, (size_t)treeDepth);
    }
    else
    {
      context.errorCallback(T("Invalid learner"));
      return LuapeLearnerPtr();
    }

    learner->setVerbose(verbose);
    return learner;
  }
};

////////////////////////////////////////

class LuapeClassificationSandBox : public LuapeClassificationWorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    /*
    juce::OwnedArray<File> dataFiles;
    if (dataDirectory.isDirectory())
      dataDirectory.findChildFiles(dataFiles, File::findFiles, false, T("*.jdb"));
    else
      dataFiles.add(new File(dataDirectory));

    for (int i = 0; i < dataFiles.size(); ++i)
    {
      File dataFile = *dataFiles[i];
      File tsFile = dataFile.getParentDirectory().getChildFile(dataFile.getFileNameWithoutExtension() + T(".txt"));
      if (dataFile.existsAsFile() && tsFile.existsAsFile())
      {
        context.enterScope(dataFile.getFileNameWithoutExtension());
        Variable res = runOnDataSet(context, dataFile, tsFile);
        context.leaveScope(res);
      }
    }
*/
    
    runOnDataSet(context, "waveform");
    runOnDataSet(context, "two-norm");
    runOnDataSet(context, "ring-norm");
    runOnDataSet(context, "vehicle");
    runOnDataSet(context, "vowel");
    runOnDataSet(context, "segment");
    runOnDataSet(context, "spambase");
    runOnDataSet(context, "satellite");
    runOnDataSet(context, "pendigits");
    runOnDataSet(context, "dig44");
    runOnDataSet(context, "letter");
    //runOnDataSet(context, "isolet");
    
    /*
    runOnDataSet(context, "liver-disorders");
    runOnDataSet(context, "glass");
    runOnDataSet(context, "ionosphere");
    runOnDataSet(context, "new-thyroid");
    runOnDataSet(context, "diabetes");
    runOnDataSet(context, "sonar");
    runOnDataSet(context, "vehicle");
    runOnDataSet(context, "wine");
    runOnDataSet(context, "wdbc");
    runOnDataSet(context, "breast-orig");*/
    return true;
  }

  void runOnDataSet(ExecutionContext& context, const String& name)
  {
    context.enterScope(name);
    File dataFile = dataDirectory.getChildFile(name + T(".jdb"));
    File tsFile;

    if (dataFile.exists())
      tsFile = dataDirectory.getChildFile(name + T(".txt"));
    else
      dataFile = dataDirectory.getChildFile(name + T(".arff"));
    Variable res = runOnDataSet(context, dataFile, tsFile);
    context.leaveScope(res);
  }

  virtual Variable runOnDataSet(ExecutionContext& context, const File& dataFile, const File& tsFile)
  {
    // load data
    DynamicClassPtr inputClass = new DynamicClass("inputs");
    DefaultEnumerationPtr labels = new DefaultEnumeration("labels");
    ContainerPtr data = loadData(context, dataFile, inputClass, labels);
    if (!data || !data->getNumElements())
      return false;

    // display data info
    size_t numVariables = inputClass->getNumMemberVariables();
    size_t numExamples = data->getNumElements();
    context.informationCallback(String((int)numExamples) + T(" examples, ") +
                                String((int)numVariables) + T(" variables, ") +
                                String((int)labels->getNumElements()) + T(" labels"));
//    context.informationCallback(String((int)trainingSize) + T(" training examples, ") + String((int)(numExamples - trainingSize)) + T(" testing examples"));

    // make splits
    std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;
    context.enterScope(T("Splits"));
    if (makeSplits(context, tsFile, inputClass, data, splits))
    {
      for (size_t i = 0; i < splits.size(); ++i)
        context.informationCallback(T("Split ") + String((int)i) + T(": train size = ") + String((int)splits[i].first->getNumElements())
                              + T(", test size = ") + String((int)splits[i].second->getNumElements()));
    }
    context.leaveScope(splits.size());
    if (!splits.size())
      return false;
    
    if (verbose)
      splits.resize(1);
    else
      splits.resize(7);
      

    TypePtr inputType = splits[0].first->getClass()->getTemplateArgument(0)->getTemplateArgument(0);
    
    static const size_t numIterations = 1000;

    LuapeLearnerPtr conditionLearner, learner;
    
    size_t Kdef = (size_t)(0.5 + sqrt((double)numVariables));

    /****
    ***** MCTS Feature Generation
    ****
    context.enterScope(T("MCTS"));
    for (size_t numIterations = 1; numIterations <= 256; numIterations *= 2)
    {
      context.enterScope(T("Num Iterations = " + String((int)numIterations)));
      context.resultCallback(T("numIterations"), numIterations);
      context.resultCallback(T("log(numIterations)"), log10((double)numIterations));
      conditionLearner = optimizerBasedSequentialWeakLearner(new SinglePlayerMCTSOptimizer(numIterations), 6);
      conditionLearner->setVerbose(verbose);
      learner = discreteAdaBoostMHLearner(conditionLearner, 100, 2);
      learner->setVerbose(verbose);
      double score = testLearner(context, learner, String::empty, inputClass, labels, splits);
      context.resultCallback(T("validationScore"), score);
      context.leaveScope(score);
    }
    context.leaveScope();*/

    /****
    ***** Baseline
    ****
    context.enterScope(T("Boosting"));
    //conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(Kdef, 2));
    conditionLearner = exactWeakLearner(inputsNodeBuilder());
    conditionLearner->setVerbose(verbose);
    learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    //learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    //learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    learner->setVerbose(verbose);
    double score = testLearner(context, learner, String::empty, inputType, labels, splits);
    context.leaveScope(score);*/

    /****
    ***** Meta-MC Test
    ****/
    std::vector<MCAlgorithmPtr> algorithms;
    //MCAlgorithmSet set(2);
    //set.getAlgorithms(algorithms);
    algorithms.push_back(rollout());
    algorithms.push_back(step(lookAhead(rollout(), 1.0), true));
    algorithms.push_back(step(lookAhead(rollout(), 1.0), false));
    algorithms.push_back(step(rollout(), true));
    algorithms.push_back(step(rollout(), false));

    context.enterScope(T("Test ") + String((int)algorithms.size()) + T(" algorithms"));
    double bestError = DBL_MAX, worstError = -DBL_MAX;
    MCAlgorithmPtr bestAlgorithm;
    MCAlgorithmPtr worstAlgorithm;
    for (size_t i = 0; i < algorithms.size(); ++i)
    {
      double error = testMCAlgorithm(context, i, algorithms[i], numVariables, inputType, labels, splits);
      if (error < bestError)
        bestError = error, bestAlgorithm = algorithms[i];
      if (error > worstError)
        worstError = error, worstAlgorithm = algorithms[i];
      context.progressCallback(new ProgressionState(i+1, algorithms.size(), T("Algorithms")));
    }
    context.resultCallback("bestError", bestError);
    context.resultCallback("bestAlgorithm", bestAlgorithm);
    context.resultCallback("worstError", worstError);
    context.resultCallback("worstAlgorithm", worstAlgorithm);
    context.leaveScope(new Pair(bestError, bestAlgorithm->toShortString()));

    /****
    ***** Nested Monte Carlo Feature Generation
    ****
    context.enterScope(T("NMC(1,1)"));
    //size_t complexity = 4;
    //    for (size_t numIterations = 1; numIterations <= 4096; numIterations *= 2)
    {
      size_t numIterations = 1;
      //      context.enterScope(T("Num Iterations = " + String((int)numIterations)));
      //context.resultCallback(T("numIterations"), numIterations);
      //context.resultCallback(T("log(numIterations)"), log10((double)numIterations));
      //size_t maxLevel = numIterations <= 16 ? 1 : 0;
      size_t level = 1;
      //for (size_t level = 0; level <= maxLevel; ++level)

      for (size_t complexity = 6; complexity <= 6; complexity += 2)
      {
        conditionLearner = optimizerBasedSequentialWeakLearner(new NestedMonteCarloOptimizer(level, numIterations), complexity, false);
        conditionLearner->setVerbose(verbose);
        //learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 5);
        //learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
        //learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
        learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
        learner->setVerbose(verbose);
        double score = testLearner(context, learner, T("RF(100) NMC(1,1,") + String((int)complexity) + T(")"), inputType, labels, splits);
        
        learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 250);
        learner->setVerbose(verbose);
        score = testLearner(context, learner, T("RF(250) NMC(1,1,") + String((int)complexity) + T(")"), inputType, labels, splits);
        //String str = T("level") + String((int)level);
        //context.resultCallback(str + T("Score"), score);
      }

      //context.leaveScope();
    }
    context.leaveScope();
    */

    /****
    ***** Iterative Feature Generation
    ****/
    /*
    // ST
    //conditionLearner = exactWeakLearner(inputsNodeBuilder());
    //LuapeLearnerPtr targetLearner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    
    // XT
    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(Kdef, 2));
    LuapeLearnerPtr targetLearner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    
    // Boosting
    //conditionLearner = exactWeakLearner(inputsNodeBuilder());
    // LuapeLearnerPtr targetLearner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    
    targetLearner->setVerbose(verbose);

    
    // ST
    //conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    //learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    
    // XT
    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    
    // Boosting
    //    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    //learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    
    learner->setVerbose(verbose);
    //testLearner(context, learner, "Baseline explore", inputClass, labels, splits);
    //testLearner(context, targetLearner, "Baseline simple", inputClass, labels, splits);
    
    learner = new RelevanceDrivenFeatureGenerationLearner(learner, 10, numVariables, targetLearner);
    learner->setVerbose(true);
    testLearner(context, learner, "RDFG explore", inputClass, labels, splits);
    //testLearner(context, targetLearner, "RDFG simple");
*/

    /****
    ***** Various Tree Methods
    ****/
/*    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    testLearner(context, learner, "Single Tree");

    conditionLearner = exactWeakLearner(inputsNodeBuilder());
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    testLearner(context, learner, "Single Tree (check)");

    
    learner = baggingLearner(learner, 100);
    testLearner(context, learner, "Tree Bagging");

    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(K, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testLearner(context, learner, "Random Subspace");

    learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testLearner(context, learner, "Random Forests");


    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(4, 2));
    conditionLearner->setVerbose(verbose);
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    learner = ensembleLearner(learner, 100);
    learner->setVerbose(verbose);
    testLearner(context, learner, "Extra Trees - K=4");

   
    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testLearner(context, learner, "Extra Trees - K=N");

    conditionLearner = randomSplitWeakLearner(inputsNodeBuilder());
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testLearner(context, learner, "Extra Trees - K=N check");


    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(K, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testLearner(context, learner, "Extra Trees Default");
    */

    /****
    ***** Monte Carlo n-variables 
    ****/

    //LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(numVariables, 2);

#if 0
    LuapeNodeBuilderPtr nodeBuilder = inputsNodeBuilder();
    nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
    //conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
    conditionLearner = exactWeakLearner(nodeBuilder);
    conditionLearner->setVerbose(verbose);
    conditionLearner = new SoftStumpWeakLearner(conditionLearner);
    conditionLearner->setVerbose(verbose);
    learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
    learner->setVerbose(verbose);
    testLearner(context, learner, "ThreeStumps Boosting - 1-var");

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str = (complexity == 2 ? T("1 variable") : String((int)complexity / 2) + T(" variables"));
      LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(numVariables, complexity);
      nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);

      //conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
      conditionLearner = exactWeakLearner(nodeBuilder);
      conditionLearner->setVerbose(verbose);
      conditionLearner = new SoftStumpWeakLearner(conditionLearner);
      conditionLearner->setVerbose(verbose);
      learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
      learner->setVerbose(verbose);

      testLearner(context, learner, "ThreeStumps AdaBoost.MH K=n - " + str);
    }
#endif // 0

    /****
    ***** Relevance driven Monte Carlo n-variables 
    ****/

    /*
    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str = String((int)complexity / 2) + T(" variables");

      double bestScore = DBL_MAX;
      context.enterScope(T("ThreeStumps AdaBoost.MH") + str);
      for (double budget = 0.25; budget <= 16.0; budget *= 2.0)
      {
	if (complexity == 2 && budget > 1.0)
	  break;
	//        double initialImportance = pow(10.0, logInitialImportance);
        
        context.enterScope(T("Budget = ") + String(budget));
        context.resultCallback(T("budget"), budget);
        //context.resultCallback(T("initialImportance"), initialImportance);
        LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder((size_t)(budget * numVariables + 0.5), complexity);
        nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
        //conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
        conditionLearner = exactWeakLearner(nodeBuilder);
        conditionLearner->setVerbose(verbose);
        learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
        learner->setVerbose(verbose);
        double validationScore = testLearner(context, learner, String::empty);
        bestScore = juce::jmin(bestScore, validationScore);
        context.leaveScope(validationScore);
      }
      context.leaveScope(bestScore);
    }*/
    return true;
  }
  
  double testMCAlgorithm(ExecutionContext& context, size_t index, MCAlgorithmPtr algorithm,
    size_t numVariables, TypePtr inputType, DefaultEnumerationPtr labels, const std::vector< std::pair< ContainerPtr, ContainerPtr > >& splits) const
  {
    context.enterScope(algorithm->toShortString());
    context.resultCallback("index", index);
    context.resultCallback("algorithm", algorithm->toShortString());
    size_t complexity = 6;
    size_t budget = complexity * (4 + numVariables);
    LuapeLearnerPtr conditionLearner = optimizerBasedSequentialWeakLearner(new MCOptimizer(algorithm, budget), complexity, false);
    conditionLearner->setVerbose(verbose);
    LuapeLearnerPtr learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    learner->setVerbose(verbose);
    double res = testLearner(context, learner, String::empty, inputType, labels, splits);
    context.resultCallback("score", res);
    context.leaveScope(res);
    return res;
  }

  double testLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name,
    TypePtr inputType, DefaultEnumerationPtr labels, const std::vector< std::pair< ContainerPtr, ContainerPtr > >& splits) const
  {
  //  Object::displayObjectAllocationInfo(std::cout);

    if (name.isNotEmpty())
      context.enterScope(name);

    // construct parallel work unit 
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(name, splits.size());
    for (size_t i = 0; i < splits.size(); ++i)
      workUnit->setWorkUnit(i, new TrainAndTestLearnerWorkUnit(this, learner->cloneAndCast<LuapeLearner>(), splits[i].first, splits[i].second, inputType, labels, "Split " + String((int)i)));
    workUnit->setProgressionUnit(T("Splits"));
    workUnit->setPushChildrenIntoStackFlag(true);

    // run parallel work unit
    ContainerPtr results = context.run(workUnit, false).getObjectAndCast<Container>();
    jassert(results->getNumElements() == splits.size());


    double res = 0.0;
    if (results->getElement(0).dynamicCast<ExecutionTrace>())
    {
      mergeFoldTraces(context, name, results);
    }
    else
    {
      ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics();
      for (size_t i = 0; i < results->getNumElements(); ++i)
        stats->push(results->getElement(i).getDouble());
      res = stats->getMean();
    }

    // compile results
    if (name.isNotEmpty())
      context.leaveScope(res);

    return res;
  }    

  void mergeFoldTraces(ExecutionContext& context, String name, ContainerPtr traces) const
  {
    size_t numFolds = traces->getNumElements();
    std::vector<ExecutionTraceNodePtr> learningNodes(numFolds);
    for (size_t i = 0; i < numFolds; ++i)
      learningNodes[i] = traces->getElement(i).getObjectAndCast<ExecutionTrace>()->getRootNode()->getSubItems()[0];

    if (learningNodes[0]->getNumSubItems() <= 2)
      return;

    size_t numIterations = learningNodes[0]->getNumSubItems() - 2;

    context.enterScope(name + T(" results"));
    for (size_t i = 0; i < numIterations; ++i)
    {
      context.enterScope(T("Iteration ") + String((int)i + 1));
      context.resultCallback(T("iteration"), i + 1);
      std::map<String, ScalarVariableStatisticsPtr> results;
      for (size_t j = 0; j < numFolds; ++j)
      {
        ExecutionTraceNodePtr resultsNode = learningNodes[j]->getSubItems()[i + 2].dynamicCast<ExecutionTraceNode>();
        if (!resultsNode)
          continue;
        std::vector<std::pair<String, Variable> > v = resultsNode->getResults();
        for (size_t k = 0; k < v.size(); ++k)
        {
          Variable value = v[k].second;
          if (value.isConvertibleToDouble())
          {
            String name = v[k].first;
            ScalarVariableStatisticsPtr& stats = results[name];
            if (!stats)
              stats = new ScalarVariableStatistics(name);
            stats->push(value.toDouble());
          }
        }
      }
      for (std::map<String, ScalarVariableStatisticsPtr>::const_iterator it = results.begin(); it != results.end(); ++it)
        context.resultCallback(it->first, it->second->getMean());
      context.leaveScope();
    }
    context.leaveScope();
  }

protected:
  friend class LuapeClassificationSandBoxClass;

  File dataDirectory;

  struct TrainAndTestLearnerWorkUnit : public WorkUnit
  {
    TrainAndTestLearnerWorkUnit(const LuapeClassificationSandBox* owner, const LuapeLearnerPtr& learner, const ContainerPtr& trainingData, const ContainerPtr& testingData,
                                const TypePtr& inputType, const DefaultEnumerationPtr& labels, const String& description)
      : owner(owner), learner(learner), trainingData(trainingData), testingData(testingData), inputType(inputType), labels(labels), description(description) {}

    virtual String toShortString() const
      {return description;}

    virtual Variable run(ExecutionContext& context)
    {
      LuapeClassifierPtr classifier = owner->createClassifier(inputType);
      if (!classifier->initialize(context, inputType, labels))
        return ScoreObjectPtr();
      return owner->trainAndTestClassifier(context, classifier, learner, trainingData, testingData);
    }

  protected:
    const LuapeClassificationSandBox* owner;
    LuapeLearnerPtr learner;
    ContainerPtr trainingData;
    ContainerPtr testingData;
    TypePtr inputType;
    DefaultEnumerationPtr labels;
    String description;
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
