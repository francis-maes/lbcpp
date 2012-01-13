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
      char* name = strtok(isFirst ? line : NULL, " \t\n");
      char* kind = strtok(NULL, " \t\n");
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

///////////////////////////////////////////////////////////////////////////////

class LuapeClassificationWorkUnit : public WorkUnit
{
public:
  LuapeClassificationWorkUnit(const File& dataFile, const File& tsFile, const LuapeLearnerPtr& learner, const String& methodName, const String& variantName)
    : dataFile(dataFile), tsFile(tsFile), learner(learner), methodName(methodName), variantName(variantName) {}
  LuapeClassificationWorkUnit() {}

  virtual Variable run(ExecutionContext& context)
  {
    context.informationCallback(T("Dataset: ") + dataFile.getFileNameWithoutExtension());
    context.informationCallback(T("Learner: ") + learner->toShortString());

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

    // run on splits
    ScalarVariableStatisticsPtr res = new ScalarVariableStatistics(T("validation"));
    for (size_t i = 0; i < splits.size(); ++i)
    {
      context.enterScope(T("Split ") + String((int)i));
      double score = runOnSplit(context, splits[i].first, splits[i].second);
      res->push(score);
      context.leaveScope(score);
    }
    return res;
  }

protected:
  friend class LuapeClassificationWorkUnitClass;

  File dataFile;
  File tsFile;
  LuapeLearnerPtr learner;
  String methodName;
  String variantName;

  DynamicClassPtr inputClass;
  DefaultEnumerationPtr labels;
  std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr labels) const
  { 
    static const bool sparseData = true;

    context.enterScope(T("Loading ") + file.getFileName());
    TextParserPtr parser = new JDBDataParser(context, file, inputClass, labels, sparseData);
    ContainerPtr res = parser->load();
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  bool makeSplits(ExecutionContext& context, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
    TextParserPtr parser = new TestingSetParser(context, tsFile, data);
    ContainerPtr splits = parser->load();
    res.resize(splits->getNumElements());
    for (size_t i = 0; i < res.size(); ++i)
    {
      PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
      res[i] = std::make_pair(split->getFirst().getObjectAndCast<Container>(), split->getSecond().getObjectAndCast<Container>());
    }
    return true;
  }

  double runOnSplit(ExecutionContext& context, const ContainerPtr& trainingData, const ContainerPtr& testingData)
  {
    LuapeClassifierPtr classifier = createClassifier(inputClass);
    if (!classifier->initialize(context, inputClass, labels))
      return DBL_MAX;
    LuapeBatchLearnerPtr batchLearner = new LuapeBatchLearner(learner);
    classifier->setBatchLearner(batchLearner);
    classifier->setEvaluator(defaultSupervisedEvaluator());
    ScoreObjectPtr score = classifier->train(context, trainingData, testingData, String::empty, false);
    return score ? score->getScoreToMinimize() : DBL_MAX;
  }

  LuapeInferencePtr createClassifier(DynamicClassPtr inputClass) const
  {
    LuapeInferencePtr res = new LuapeClassifier();
    size_t n = inputClass->getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      VariableSignaturePtr variable = inputClass->getMemberVariable(i);
      res->addInput(variable->getType(), variable->getName());
    }

    res->addFunction(addDoubleLuapeFunction());
    res->addFunction(subDoubleLuapeFunction());
    res->addFunction(mulDoubleLuapeFunction());
    res->addFunction(divDoubleLuapeFunction());
    return res;
  }
};

class LaunchLuapeClassification : public WorkUnit, public ExecutionContextCallback
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    std::vector<String> to;
    to.push_back(T("jbecker@nic3"));
    to.push_back(T("fmaes@nic3"));
    to.push_back(T("amarcos@nic3"));
    
    distributedContext = distributedExecutionContext(context, "monster24.montefiore.ulg.ac.be", 1664, "FormulaBoosting", "Francis", to, fixedResourceEstimator(1, 2048, 24));
    if (outputFile.existsAsFile())
      outputFile.deleteFile();
    ostr = outputFile.createOutputStream();

    // data files
    juce::OwnedArray<File> dataFiles;
    dataDirectory.findChildFiles(dataFiles, File::findFiles, false, T("*.jdb"));

    // methods
    std::vector< std::pair<String, LearnerConstructor> > methods;
    methods.push_back(std::make_pair("SingleTree", &LaunchLuapeClassification::singleTreeLearner));

    methods.push_back(std::make_pair("TreeEnsemble", &LaunchLuapeClassification::treeEnsembleLearner));
    methods.push_back(std::make_pair("TreeEnsembleStochasticDeep", &LaunchLuapeClassification::treeEnsembleStochasticDeepLearner));
    methods.push_back(std::make_pair("TreeEnsembleDeterministicDeep", &LaunchLuapeClassification::treeEnsembleDeterministicDeepLearner));

    methods.push_back(std::make_pair("TreeBagging", &LaunchLuapeClassification::treeBaggingLearner));
    methods.push_back(std::make_pair("TreeBaggingStochasticDeep", &LaunchLuapeClassification::treeBaggingStochasticDeepLearner));
    methods.push_back(std::make_pair("TreeBaggingDeterministicDeep", &LaunchLuapeClassification::treeBaggingDeterministicDeepLearner));

    methods.push_back(std::make_pair("AdaBoostMH1", &LaunchLuapeClassification::singleStumpAdaBoostMHLearner));
    methods.push_back(std::make_pair("AdaBoostMH1StochasticDeep", &LaunchLuapeClassification::singleStumpAdaBoostMHStochasticDeepLearner));
    methods.push_back(std::make_pair("AdaBoostMH1DeterministicDeep", &LaunchLuapeClassification::singleStumpAdaBoostMHDeterministicDeepLearner));

    methods.push_back(std::make_pair("AdaBoostMH2", &LaunchLuapeClassification::treeDepth2AdaBoostMHLearner));
    methods.push_back(std::make_pair("AdaBoostMH2StochasticDeep", &LaunchLuapeClassification::treeDepth2AdaBoostMHStochasticDeepLearner));
    methods.push_back(std::make_pair("AdaBoostMH2DeterministicDeep", &LaunchLuapeClassification::treeDepth2AdaBoostMHDeterministicDeepLearner));
    
    methods.push_back(std::make_pair("AdaBoostMH3", &LaunchLuapeClassification::treeDepth3AdaBoostMHLearner));
    methods.push_back(std::make_pair("AdaBoostMH3StochasticDeep", &LaunchLuapeClassification::treeDepth3AdaBoostMHStochasticDeepLearner));
    methods.push_back(std::make_pair("AdaBoostMH3DeterministicDeep", &LaunchLuapeClassification::treeDepth3AdaBoostMHDeterministicDeepLearner));

    methods.push_back(std::make_pair("AdaBoostMH4", &LaunchLuapeClassification::treeDepth4AdaBoostMHLearner));
    methods.push_back(std::make_pair("AdaBoostMH5", &LaunchLuapeClassification::treeDepth5AdaBoostMHLearner));
   
    context.informationCallback(String(dataFiles.size()) + T(" data files"));
    context.informationCallback(String((int)methods.size()) + T(" methods"));
    for (int i = 0; i < dataFiles.size(); ++i)
    {
      File dataFile = *dataFiles[i];
      File tsFile = dataFile.getParentDirectory().getChildFile(dataFile.getFileNameWithoutExtension() + T(".txt"));

      for (size_t j = 0; j < methods.size(); ++j)
      {
        String method = methods[j].first;
        LearnerConstructor learnerConstructor = methods[j].second;
        launchWorkUnits(context, dataFile, tsFile, method, learnerConstructor);
      }
    }

    while (true)
    {
      distributedContext->waitUntilAllWorkUnitsAreDone(1000);
      distributedContext->flushCallbacks();
      std::cout << "." << std::flush;
    }
    return true;
  }

  typedef LuapeLearnerPtr (LaunchLuapeClassification::*LearnerConstructor)(LuapeLearnerPtr conditionLearner) const;

  void launchWorkUnits(ExecutionContext& context, const File& dataFile, const File& tsFile, const String& method, LearnerConstructor learnerConstructor)
  {
    inputClass = new DynamicClass("inputs");
    labels = new DefaultEnumeration("labels");

    TextParserPtr parser = new JDBDataParser(context, dataFile, inputClass, labels, true);
    ContainerPtr res = parser->load();
    size_t numVariables = inputClass->getNumMemberVariables();
    static const int minExamplesForLaminating = 10;

    std::vector< std::pair<String, LuapeLearnerPtr> > variants;
    variants.push_back(std::make_pair("1VarFullRandom",
      randomSplitWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2))));
    variants.push_back(std::make_pair("1VarRandomSubspace",
      exactWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2))));
    variants.push_back(std::make_pair("1VarFull",
      exactWeakLearner(randomSequentialNodeBuilder(numVariables, 2))));
    variants.push_back(std::make_pair("1VarFullCheck",
      exactWeakLearner(inputsNodeBuilder())));

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str((int)complexity / 2);
      str += T("Var");
      variants.push_back(std::make_pair(str + T("FullRandom"),
          randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, complexity))));
      variants.push_back(std::make_pair(str + T("RandomSubspace"),
          exactWeakLearner(randomSequentialNodeBuilder(numVariables, complexity))));
      variants.push_back(std::make_pair(str + T("Laminating"),
          laminatingWeakLearner(randomSequentialNodeBuilder(numVariables, complexity), (double)numVariables, minExamplesForLaminating)));
    }

    for (size_t i = 0; i < variants.size(); ++i)
    {
      LuapeLearnerPtr learner = (this->*learnerConstructor)(variants[i].second);
      launchWorkUnit(context, dataFile, tsFile, learner, method, variants[i].first);
    }
  }

  void launchWorkUnit(ExecutionContext& context, const File& dataFile, const File& tsFile, const LuapeLearnerPtr& learner, const String& method, const String& variant)
  {
    WorkUnitPtr workUnit = new LuapeClassificationWorkUnit(dataFile, tsFile, learner, method, variant);
    context.informationCallback(T("WorkUnit: ") + workUnit->toShortString());  
    distributedContext->pushWorkUnit(workUnit, this); 
  }

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    std::cout << "Result: " << result.toShortString() << std::endl;
    ClassPtr cl = workUnit->getClass();
    File dataFile = workUnit->getVariable(cl->findMemberVariable("dataFile")).getFile();
    String method = workUnit->getVariable(cl->findMemberVariable("methodName")).getString();
    String variant = workUnit->getVariable(cl->findMemberVariable("variantName")).getString();
    if (result.isBoolean())
    {
      std::cerr << "Null results" << std::endl;
      return;
    }
    ScalarVariableStatisticsPtr stats = result.getObjectAndCast<ScalarVariableStatistics>();
    if (!stats)
      {
	std::cerr << "Bad results" << std::endl;
	return;
      }

    String info = dataFile.getFileNameWithoutExtension() + T(" ") + method + T(" ") + variant + T(" ") +
        String(stats->getMean()) + T(" ") + String(stats->getStandardDeviation());
    std::cout << info << std::endl;
    *ostr << info << "\n";
    ostr->flush();
  }

protected:
  friend class LaunchLuapeClassificationClass;

  File dataDirectory;
  File outputFile;

  juce::OutputStream* ostr;

  ExecutionContextPtr distributedContext;

  DynamicClassPtr inputClass;
  DefaultEnumerationPtr labels;

  LuapeLearnerPtr singleTreeLearner(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);}

  LuapeLearnerPtr treeBaggingLearner(LuapeLearnerPtr conditionLearner) const
    {return baggingLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeBaggingStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return baggingLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, false), 100);
  }
  
  LuapeLearnerPtr treeBaggingDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return baggingLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, true), 100);
  }

  LuapeLearnerPtr treeEnsembleLearner(LuapeLearnerPtr conditionLearner) const
    {return ensembleLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeEnsembleStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return ensembleLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, false), 100);
  }
  
  LuapeLearnerPtr treeEnsembleDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return ensembleLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, true), 100);
  }

  LuapeLearnerPtr singleStumpAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr singleStumpAdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000);
  }

  LuapeLearnerPtr singleStumpAdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000);
  }

  LuapeLearnerPtr treeDepth2AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 2);}
  
  LuapeLearnerPtr treeDepth2AdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000, 2);
  }

  LuapeLearnerPtr treeDepth2AdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000, 2);
  }

  LuapeLearnerPtr treeDepth3AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth3AdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000, 3);
  }

  LuapeLearnerPtr treeDepth3AdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000, 3);
  }

  LuapeLearnerPtr treeDepth4AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 4);}

  LuapeLearnerPtr treeDepth5AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 5);}
};


/////////////////////////////////////////////////////////////////////////////////////

class LuapeClassificationSandBox : public WorkUnit
{
public:
  LuapeClassificationSandBox() : maxExamples(0), trainingSize(0), numRuns(0), verbose(false) {}

  typedef LuapeLearnerPtr (LuapeClassificationSandBox::*LearnerConstructor)(LuapeLearnerPtr conditionLearner) const;

  LuapeLearnerPtr singleTreeLearner(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);}

  LuapeLearnerPtr treeBaggingLearner(LuapeLearnerPtr conditionLearner) const
    {return baggingLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeBaggingStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return baggingLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, false), 100);
  }
  
  LuapeLearnerPtr treeBaggingDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return baggingLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, true), 100);
  }

  LuapeLearnerPtr treeEnsembleLearner(LuapeLearnerPtr conditionLearner) const
    {return ensembleLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeEnsembleStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return ensembleLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, false), 100);
  }
  
  LuapeLearnerPtr treeEnsembleDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return ensembleLearner(addActiveVariablesLearner(singleTreeLearner(conditionLearner), numVariables, true), 100);
  }

  LuapeLearnerPtr singleStumpAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr singleStumpAdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000);
  }

  LuapeLearnerPtr singleStumpAdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000);
  }

  LuapeLearnerPtr treeDepth2AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 2);}
  
  LuapeLearnerPtr treeDepth2AdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000, 2);
  }

  LuapeLearnerPtr treeDepth2AdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000, 2);
  }

  LuapeLearnerPtr treeDepth3AdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth3AdaBoostMHStochasticDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, false), 1000, 3);
  }

  LuapeLearnerPtr treeDepth3AdaBoostMHDeterministicDeepLearner(LuapeLearnerPtr conditionLearner) const
  {
    size_t numVariables = inputClass->getNumMemberVariables();
    return discreteAdaBoostMHLearner(addActiveVariablesLearner(conditionLearner, numVariables, true), 1000, 3);
  }

  LuapeLearnerPtr treeDepth4DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 4);}

  LuapeLearnerPtr treeDepth5DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 5);}


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

    LuapeLearnerPtr conditionLearner, learner;
    
    size_t K = (size_t)(0.5 + sqrt((double)numVariables));

    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Single Tree");
/*
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

    for (size_t complexity = 2; complexity <= 8; complexity += 2)
    {
      String str = (complexity == 2 ? T("1 variable") : String((int)complexity / 2) + T(" variables"));
      LuapeNodeBuilderPtr nodeBuilder = randomSequentialNodeBuilder(numVariables, complexity);
      conditionLearner = randomSplitWeakLearner(nodeBuilder);
      conditionLearner->setVerbose(verbose);
      learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
      learner->setVerbose(verbose);
      learner = ensembleLearner(learner, 100);
      learner->setVerbose(verbose);
      testConditionLearner(context, learner, "Extra Trees - " + str);
    }*/

    for (size_t complexity = 2; complexity <= 8; complexity += 2)
    {
      String str = (complexity == 2 ? T("1 variable") : String((int)complexity / 2) + T(" variables"));

      double bestScore = DBL_MAX;
      context.enterScope(T("Deep Extra Trees - ") + str);
      for (double logInitialImportance = -3.0; logInitialImportance <= 3.0; logInitialImportance += 0.5)
      {
        double initialImportance = pow(10.0, logInitialImportance);
        
        context.enterScope(T("Initial importance = ") + String(initialImportance));
        context.resultCallback(T("logInitialImportance"), logInitialImportance);
        //context.resultCallback(T("initialImportance"), initialImportance);
        LuapeNodeBuilderPtr nodeBuilder = biasedRandomSequentialNodeBuilder(numVariables, complexity, initialImportance);
        conditionLearner = randomSplitWeakLearner(nodeBuilder);
        conditionLearner->setVerbose(verbose);
        learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
        learner->setVerbose(verbose);
        learner = ensembleLearner(learner, 100);
        learner->setVerbose(verbose);
        double validationScore = testConditionLearner(context, learner, String::empty);
        bestScore = juce::jmin(bestScore, validationScore);
        context.leaveScope(validationScore);
      }
      context.leaveScope(bestScore);
    }
    return true;

    /*

    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner->setVerbose(verbose);
    learner = addActiveVariablesLearner(learner, numVariables, false);
    learner->setVerbose(verbose);
    learner = ensembleLearner(learner, 100);
    learner->setVerbose(verbose);
    testConditionLearner(context, learner, "Extra Trees - Two variables - stochastic reinjected");

    learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    learner = ensembleLearner(addActiveVariablesLearner(learner, numVariables, true), 100);
    testConditionLearner(context, learner, "Extra Trees - Two variables - deterministic reinjected");

    return true;*/

    testLearners(context, &LuapeClassificationSandBox::singleTreeLearner, T("Single tree"));

    testLearners(context, &LuapeClassificationSandBox::treeEnsembleLearner, T("Tree Ensemble"));
    testLearners(context, &LuapeClassificationSandBox::treeEnsembleStochasticDeepLearner, T("Tree Ensemble - Stochastic Deep"));
    testLearners(context, &LuapeClassificationSandBox::treeEnsembleDeterministicDeepLearner, T("Tree Ensemble - Deterministic Deep"));

    testLearners(context, &LuapeClassificationSandBox::treeBaggingLearner, T("Tree Bagging"));
    testLearners(context, &LuapeClassificationSandBox::treeBaggingStochasticDeepLearner, T("Tree Bagging - Stochastic Deep"));
    testLearners(context, &LuapeClassificationSandBox::treeBaggingDeterministicDeepLearner, T("Tree Bagging - Deterministic Deep"));

    testLearners(context, &LuapeClassificationSandBox::singleStumpAdaBoostMHLearner, T("Single stump AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpAdaBoostMHStochasticDeepLearner, T("Single stump AdaBoost.MH - Stochastic Deep"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpAdaBoostMHDeterministicDeepLearner, T("Single stump AdaBoost.MH - Deterministic Deep"));

    testLearners(context, &LuapeClassificationSandBox::treeDepth2AdaBoostMHLearner, T("Tree depth 2 AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth2AdaBoostMHStochasticDeepLearner, T("Tree depth 2 AdaBoost.MH - Stochastic Deep"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth2AdaBoostMHDeterministicDeepLearner, T("Tree depth 2 AdaBoost.MH - Deterministic Deep"));

    testLearners(context, &LuapeClassificationSandBox::treeDepth3AdaBoostMHLearner, T("Tree depth 3 AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3AdaBoostMHStochasticDeepLearner, T("Tree depth 3 AdaBoost.MH - Stochastic Deep"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3AdaBoostMHDeterministicDeepLearner, T("Tree depth 3 AdaBoost.MH - Deterministic Deep"));

    testLearners(context, &LuapeClassificationSandBox::treeDepth4DiscreteAdaBoostMHLearner, T("Tree depth 4 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth5DiscreteAdaBoostMHLearner, T("Tree depth 5 discrete AdaBoost.MH"));
    return true;
  }

  void testLearners(ExecutionContext& context, LearnerConstructor learnerConstructor, const String& name) const
  {
    static const int minExamplesForLaminating = 10;
    size_t numVariables = inputClass->getNumMemberVariables();

    context.enterScope(name);
    
    ScalarVariableStatistics scoreStats;
    LuapeLearnerPtr weakLearner;

    size_t K = (size_t)(0.5 + sqrt((double)numVariables));

    weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(K, 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random + randomsplit"), scoreStats);

    weakLearner = exactWeakLearner(randomSequentialNodeBuilder(K, 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random subspace"), scoreStats);

    weakLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable full"), scoreStats);

    weakLearner = exactWeakLearner(inputsNodeBuilder());
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable full (check)"), scoreStats);

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str((int)complexity / 2);
      str += T("-variables ");
      weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random subspace + random threshold"), scoreStats);

      weakLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random subspace"), scoreStats);

      weakLearner = laminatingWeakLearner(randomSequentialNodeBuilder(numVariables, complexity), (double)numVariables, minExamplesForLaminating);
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("laminating"), scoreStats);
    }
    context.leaveScope(new Pair(scoreStats.getMinimum(), scoreStats.getMean()));
  }

  void testConditionLearner(ExecutionContext& context, LearnerConstructor learnerConstructor, const LuapeLearnerPtr& weakLearner, const String& name, ScalarVariableStatistics& scoreStats) const
  {
    weakLearner->setVerbose(verbose);
    LuapeLearnerPtr learner = (this->*learnerConstructor)(weakLearner);
    learner->setVerbose(verbose);
    scoreStats.push(testConditionLearner(context, learner, name));
  }

  double testConditionLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name) const
  {
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
      LuapeBatchLearnerPtr batchLearner = new LuapeBatchLearner(learner);
      classifier->setBatchLearner(batchLearner);
      classifier->setEvaluator(defaultSupervisedEvaluator());
      ScoreObjectPtr score = classifier->train(context, trainingData, testingData, String::empty, false);
      return score ? score->getScoreToMinimize() : DBL_MAX;
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

      res->addFunction(addDoubleLuapeFunction());
      res->addFunction(subDoubleLuapeFunction());
      res->addFunction(mulDoubleLuapeFunction());
      res->addFunction(divDoubleLuapeFunction());
      return res;
    }
  };
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
