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
# include <lbcpp/Luape/LuapeLearner.h>
# include <lbcpp/Luape/LuapeBatchLearner.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Data/Stream.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/Numerical.h> // for lbcpp::convertSupervisionVariableToEnumValue
# include "LuapeSoftStump.h"

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
        context.errorCallback(T("Invalid index ") + String(token) + T(" (num examples = ") + String((int)data->getNumElements()) + T(")"));
        return false;
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

class LuapeClassificationSandBox : public WorkUnit
{
public:
  LuapeClassificationSandBox() : maxExamples(0), trainingSize(0), numRuns(0), verbose(false) {}

  virtual Variable run(ExecutionContext& context)
  {
    // load data
    inputClass = new DynamicClass("inputs");
    labels = new DefaultEnumeration("labels");
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
    context.enterScope(T("Splits"));
    if (makeSplits(context, data, splits))
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

    static const size_t numIterations = 1000;

    LuapeLearnerPtr conditionLearner, learner;
    
    size_t Kdef = (size_t)(0.5 + sqrt((double)numVariables));


/*    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Single Tree");

    conditionLearner = exactWeakLearner(inputsNodeBuilder());
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Single Tree (check)");

    
    learner = baggingLearner(learner, 100);
    testConditionLearner(context, learner, "Tree Bagging");

    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(K, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testConditionLearner(context, learner, "Random Subspace");

    learner = baggingLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testConditionLearner(context, learner, "Random Forests");


    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(4, 2));
    conditionLearner->setVerbose(verbose);
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    learner = ensembleLearner(learner, 100);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Extra Trees - K=4");

   
    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testConditionLearner(context, learner, "Extra Trees - K=N");

    conditionLearner = randomSplitWeakLearner(inputsNodeBuilder());
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testConditionLearner(context, learner, "Extra Trees - K=N check");


    conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(K, 2));
    learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
    testConditionLearner(context, learner, "Extra Trees Default");
    */

   
    //LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(numVariables, 2);

    LuapeNodeBuilderPtr nodeBuilder = inputsNodeBuilder();
    nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
    //conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
    conditionLearner = exactWeakLearner(nodeBuilder);
    conditionLearner->setVerbose(verbose);
    conditionLearner = new SoftStumpWeakLearner(conditionLearner, 10.0);
    conditionLearner->setVerbose(verbose);
    learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Soft Stump Boosting");

/*
    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str = (complexity == 2 ? T("1 variable") : String((int)complexity / 2) + T(" variables"));
      LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(numVariables * 10, complexity);
      nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);

      conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
      //conditionLearner = exactWeakLearner(nodeBuilder);
      conditionLearner->setVerbose(verbose);
      learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
      learner->setVerbose(verbose);

      testConditionLearner(context, learner, "ThreeStumps AdaBoost.MH Laminating K=n - " + str);
    }

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str = String((int)complexity / 2) + T(" variables");

      double bestScore = DBL_MAX;
      context.enterScope(T("Deep ThreeStumps AdaBoost.MH Laminating K=n - ") + str);
      for (double logInitialImportance = -1.0; logInitialImportance <= 4.5; logInitialImportance += 0.5)
      {
        double initialImportance = pow(10.0, logInitialImportance);
        
        context.enterScope(T("Initial importance = ") + String(initialImportance));
        context.resultCallback(T("logInitialImportance"), logInitialImportance);
        //context.resultCallback(T("initialImportance"), initialImportance);
        LuapeNodeBuilderPtr nodeBuilder = biasedRandomSequentialNodeBuilder(numVariables * 10, complexity, initialImportance);
        nodeBuilder = compositeNodeBuilder(singletonNodeBuilder(new LuapeConstantNode(true)), nodeBuilder);
        conditionLearner = laminatingWeakLearner(nodeBuilder, (double)numVariables, 10);
        //conditionLearner = exactWeakLearner(nodeBuilder);
        conditionLearner->setVerbose(verbose);
        learner = discreteAdaBoostMHLearner(conditionLearner, numIterations, 2);
        learner->setVerbose(verbose);
        double validationScore = testConditionLearner(context, learner, String::empty);
        bestScore = juce::jmin(bestScore, validationScore);
        context.leaveScope(validationScore);
      }
      context.leaveScope(bestScore);
    }*/
    return true;
  }

  double testConditionLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name) const
  {
  //  Object::displayObjectAllocationInfo(std::cout);

    if (name.isNotEmpty())
      context.enterScope(name);

    // construct parallel work unit 
    CompositeWorkUnitPtr workUnit = new CompositeWorkUnit(name, splits.size());
    for (size_t i = 0; i < splits.size(); ++i)
      workUnit->setWorkUnit(i, new TrainAndTestLearnerWorkUnit(learner->cloneAndCast<LuapeLearner>(), splits[i].first, splits[i].second, inputClass, labels, "Split " + String((int)i)));
    workUnit->setProgressionUnit(T("Splits"));
    workUnit->setPushChildrenIntoStackFlag(true);

    // run parallel work unit
    ContainerPtr results = context.run(workUnit, false).getObjectAndCast<Container>();
    jassert(results->getNumElements() == splits.size());

    // compile results
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics(T("error"));
    for (size_t i = 0; i < splits.size(); ++i)
      stats->push(results->getElement(i).getDouble());
    context.informationCallback(T("Validation score: ") + stats->toShortString());
    if (name.isNotEmpty())
      context.leaveScope(stats);
    return stats->getMean();
  }

protected:
  friend class LuapeClassificationSandBoxClass;

  File dataFile;
  size_t maxExamples;
  File tsFile;
  size_t trainingSize;
  size_t numRuns;
  bool verbose;

  DynamicClassPtr inputClass;
  DefaultEnumerationPtr labels;
  std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;

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

  bool makeSplits(ExecutionContext& context, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
    if (tsFile.existsAsFile())
    {
      TextParserPtr parser = new TestingSetParser(context, tsFile, data);
      ContainerPtr splits = parser->load();
      res.resize(splits->getNumElements());
      for (size_t i = 0; i < res.size(); ++i)
      {
        PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
        res[i] = std::make_pair(split->getFirst().getObjectAndCast<Container>(), split->getSecond().getObjectAndCast<Container>());
      }
    }
    else
    { 
      if (trainingSize >= data->getNumElements())
      {
        context.errorCallback(T("Training size is too big"));
        return false;
      }

      res.resize(numRuns);
      for (size_t i = 0; i < numRuns; ++i)
      {
        ContainerPtr randomized = data->randomize();
        ContainerPtr training = randomized->range(0, trainingSize);
        ContainerPtr testing = randomized->range(trainingSize, randomized->getNumElements());
        res[i] = std::make_pair(training, testing);
      }
    }
    return true;
  }

  struct TrainAndTestLearnerWorkUnit : public WorkUnit
  {
    TrainAndTestLearnerWorkUnit(const LuapeLearnerPtr& learner, const ContainerPtr& trainingData, const ContainerPtr& testingData,
                                const DynamicClassPtr& inputClass, const DefaultEnumerationPtr& labels, const String& description)
      : learner(learner), trainingData(trainingData), testingData(testingData), inputClass(inputClass), labels(labels), description(description) {}

    virtual String toShortString() const
      {return description;}

    virtual Variable run(ExecutionContext& context)
    {
      LuapeClassifierPtr classifier = createClassifier(inputClass);
      if (!classifier->initialize(context, inputClass, labels))
        return ScoreObjectPtr();

      classifier->setSamples(context, trainingData.staticCast<ObjectVector>()->getObjects(), testingData.staticCast<ObjectVector>()->getObjects());
      //classifier->getTrainingCache()->disableCaching();
      //classifier->getValidationCache()->disableCaching();
      LuapeNodePtr rootNode = learner->createInitialNode(context, classifier);
      classifier->setRootNode(context, rootNode);    
      learner->learn(context, rootNode, classifier, classifier->getTrainingCache()->getAllIndices());
      return classifier->evaluatePredictions(context, classifier->getValidationPredictions(), classifier->getValidationSupervisions());
/*
      LuapeBatchLearnerPtr batchLearner = new LuapeBatchLearner(learner);
      classifier->setBatchLearner(batchLearner);
      classifier->setEvaluator(defaultSupervisedEvaluator());
      ScoreObjectPtr score = classifier->train(context, trainingData, testingData, String::empty, false);
      return score ? score->getScoreToMinimize() : DBL_MAX;*/
    }

  protected:
    LuapeLearnerPtr learner;
    ContainerPtr trainingData;
    ContainerPtr testingData;
    DynamicClassPtr inputClass;
    DefaultEnumerationPtr labels;
    String description;

    LuapeInferencePtr createClassifier(DynamicClassPtr inputClass) const
    {
      LuapeInferencePtr res = new LuapeClassifier();
      size_t n = inputClass->getNumMemberVariables();
      for (size_t i = 0; i < n; ++i)
      {
        VariableSignaturePtr variable = inputClass->getMemberVariable(i);
        res->addInput(variable->getType(), variable->getName());
      }

      //res->addFunction(logDoubleLuapeFunction());
      //res->addFunction(sqrtDoubleLuapeFunction());

      res->addFunction(addDoubleLuapeFunction());
      res->addFunction(subDoubleLuapeFunction());
      res->addFunction(mulDoubleLuapeFunction());
      res->addFunction(divDoubleLuapeFunction());
      //res->addFunction(minDoubleLuapeFunction());
      //res->addFunction(maxDoubleLuapeFunction());
      return res;
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
