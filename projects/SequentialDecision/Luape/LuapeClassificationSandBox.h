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

class LuapeClassificationSandBox : public WorkUnit
{
public:
  LuapeClassificationSandBox() : maxExamples(0), verbose(false) {}

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
    runOnDataSet(context, "isolet");

    return true;
  }

  void runOnDataSet(ExecutionContext& context, const String& name)
  {
    context.enterScope(name);
    File dataFile = dataDirectory.getChildFile(name + T(".jdb"));
    File tsFile = dataDirectory.getChildFile(name + T(".txt"));
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
    if (makeSplits(context, tsFile, data, splits))
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
    //else
    //  splits.resize(7);
      

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
    ***** Nested Monte Carlo Feature Generation
    ****/
    context.enterScope(T("Nested MC"));
    size_t complexity = 6;
    //for (size_t numIterations = 1; numIterations <= numVariables * complexity * 5; numIterations *= 2)
    {size_t numIterations = 1;
      //context.enterScope(T("Num Iterations = " + String((int)numIterations)));
      //context.resultCallback(T("numIterations"), numIterations);
      //context.resultCallback(T("log(numIterations)"), log10((double)numIterations));
      //size_t maxLevel = numIterations <= 16 ? 1 : 0;
      size_t level = 1;
      //for (size_t level = 0; level <= maxLevel; ++level)
      {
        conditionLearner = optimizerBasedSequentialWeakLearner(new NestedMonteCarloOptimizer(level, numIterations), complexity);
        conditionLearner->setVerbose(verbose);
        //learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 2);
        //learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
        learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 100);
        learner->setVerbose(verbose);
        double score = testLearner(context, learner, T("Level ") + String((int)level), inputClass, labels, splits);
        //String str = T("level") + String((int)level);
        //context.resultCallback(str + T("Score"), score);
      }
      //context.leaveScope();
    }
    context.leaveScope();
    

    /****
    ***** Iterative Feature Generation
    ****/
    /*
    // ST
    //conditionLearner = exactWeakLearner(inputsNodeBuilder());
    //LuapeLearnerPtr targetLearner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    
    // XT
    //conditionLearner = randomSplitWeakLearner(inputsNodeBuilder());
    //LuapeLearnerPtr targetLearner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 5);
    
    // Boosting
    conditionLearner = exactWeakLearner(inputsNodeBuilder());
    LuapeLearnerPtr targetLearner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    
    targetLearner->setVerbose(verbose);

    
    // ST
    //conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    //learner = treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);
    
    // XT
    //conditionLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    //learner = ensembleLearner(treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0), 5);
    
    // Boosting
    conditionLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, 4));
    learner = discreteAdaBoostMHLearner(conditionLearner, 1000, 1);
    
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

  double testLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name,
    DynamicClassPtr inputClass, DefaultEnumerationPtr labels, const std::vector< std::pair< ContainerPtr, ContainerPtr > >& splits) const
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
  size_t maxExamples;
  bool verbose;

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

  bool makeSplits(ExecutionContext& context, const File& tsFile, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
   // if (tsFile.existsAsFile())
    {
      TextParserPtr parser = new TestingSetParser(context, tsFile, data);
      ContainerPtr splits = parser->load(verbose ? 1 : 0);
      res.resize(splits->getNumElements());
      for (size_t i = 0; i < res.size(); ++i)
      {
        PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
        res[i] = std::make_pair(split->getFirst().getObjectAndCast<Container>(), split->getSecond().getObjectAndCast<Container>());
      }
    }
    /*else
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
    }*/
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
      if (!trainingData->getNumElements())
        return ScoreObjectPtr();
    
      classifier->setSamples(context, trainingData.staticCast<ObjectVector>()->getObjects(), testingData.staticCast<ObjectVector>()->getObjects());
      classifier->getTrainingCache()->disableCaching();
      classifier->getValidationCache()->disableCaching();

      ExecutionTracePtr trace = new ExecutionTrace(T("hop"));
      ExecutionCallbackPtr makeTraceCallback = makeTraceExecutionCallback(trace);
      context.appendCallback(makeTraceCallback);
      learner->learn(context, classifier);
      context.removeCallback(makeTraceCallback);

      //return trace;
      return classifier->evaluatePredictions(context, classifier->getValidationPredictions(), classifier->getValidationSupervisions());
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

/*      res->addFunction(logDoubleLuapeFunction());
      res->addFunction(sqrtDoubleLuapeFunction());
      res->addFunction(minDoubleLuapeFunction());
      res->addFunction(maxDoubleLuapeFunction());*/
      
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
