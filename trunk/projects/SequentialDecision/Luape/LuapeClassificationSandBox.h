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

class LuapeClassificationSandBox : public WorkUnit
{
public:
  LuapeClassificationSandBox() : maxExamples(0), trainingSize(0), numRuns(0), verbose(false) {}

  typedef LuapeLearnerPtr (LuapeClassificationSandBox::*LearnerConstructor)(LuapeLearnerPtr conditionLearner) const;

  LuapeLearnerPtr singleStumpDiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr singleStumpRealAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return realAdaBoostMHLearner(conditionLearner, 1000);}

  LuapeLearnerPtr treeDepth2DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 2);}

  LuapeLearnerPtr treeDepth3DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth3RealAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return realAdaBoostMHLearner(conditionLearner, 1000, 3);}

  LuapeLearnerPtr treeDepth4DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 100, 4);}

  LuapeLearnerPtr treeDepth5DiscreteAdaBoostMHLearner(LuapeLearnerPtr conditionLearner) const
    {return discreteAdaBoostMHLearner(conditionLearner, 100, 5);}

  LuapeLearnerPtr singleTreeLearner(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(false), conditionLearner, 2, 0);}

  LuapeLearnerPtr singleTreeLearnerNormalizedIG(LuapeLearnerPtr conditionLearner) const
    {return treeLearner(new InformationGainLearningObjective(true), conditionLearner, 2, 0);}

  LuapeLearnerPtr treeEnsembleLearner(LuapeLearnerPtr conditionLearner) const
    {return ensembleLearner(singleTreeLearner(conditionLearner), 100);}

  LuapeLearnerPtr treeBaggingLearner(LuapeLearnerPtr conditionLearner) const
    {return baggingLearner(singleTreeLearner(conditionLearner), 100);}

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
    if (trainingSize >= numExamples)
    {
      context.errorCallback(T("Training size is too big"));
      return false;
    }
    context.informationCallback(String((int)trainingSize) + T(" training examples, ") + String((int)(numExamples - trainingSize)) + T(" testing examples"));

    // sample splits
    splits.resize(numRuns);
    for (size_t i = 0; i < numRuns; ++i)
    {
      ContainerPtr randomized = data->randomize();
      ContainerPtr training = randomized->range(0, trainingSize);
      ContainerPtr testing = randomized->range(trainingSize, randomized->getNumElements());
      splits[i] = std::make_pair(training, testing);
    }

    testLearners(context, &LuapeClassificationSandBox::singleTreeLearner, T("Single tree"));
    testLearners(context, &LuapeClassificationSandBox::singleTreeLearnerNormalizedIG, T("Single tree - normalized IG"));
    testLearners(context, &LuapeClassificationSandBox::treeBaggingLearner, T("Tree Bagging"));
    testLearners(context, &LuapeClassificationSandBox::treeEnsembleLearner, T("Tree Ensemble"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpDiscreteAdaBoostMHLearner, T("Single stump discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::singleStumpRealAdaBoostMHLearner, T("Single stump real AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth2DiscreteAdaBoostMHLearner, T("Tree depth 2 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3DiscreteAdaBoostMHLearner, T("Tree depth 3 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth3RealAdaBoostMHLearner, T("Tree depth 3 real AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth4DiscreteAdaBoostMHLearner, T("Tree depth 4 discrete AdaBoost.MH"));
    testLearners(context, &LuapeClassificationSandBox::treeDepth5DiscreteAdaBoostMHLearner, T("Tree depth 5 discrete AdaBoost.MH"));
    return true;
  }

  void testConditionLearner(ExecutionContext& context, LearnerConstructor learnerConstructor, const LuapeLearnerPtr& weakLearner, const String& name, ScalarVariableStatistics& scoreStats) const
  {
    weakLearner->setVerbose(verbose);
    LuapeLearnerPtr learner = (this->*learnerConstructor)(weakLearner);
    learner->setVerbose(verbose);
    scoreStats.push(testConditionLearner(context, learner, name));
  }

  void testLearners(ExecutionContext& context, LearnerConstructor learnerConstructor, const String& name) const
  {
    static const int minExamplesForLaminating = 10;
    size_t numVariables = inputClass->getNumMemberVariables();

    context.enterScope(name);
    
    ScalarVariableStatistics scoreStats;
    LuapeLearnerPtr weakLearner;

    bool isExtraTrees = name.startsWith(T("Extra trees"));

    weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random + randomsplit"), scoreStats);

    weakLearner = exactWeakLearner(randomSequentialNodeBuilder((size_t)sqrt((double)numVariables), 2));
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable random"), scoreStats);

    weakLearner = exactWeakLearner(inputsNodeBuilder());
    testConditionLearner(context, learnerConstructor, weakLearner, T("Single-variable full"), scoreStats);

    for (size_t complexity = 4; complexity <= 8; complexity += 2)
    {
      String str(complexity / 2);
      str += T("-variables ");
      weakLearner = randomSplitWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random + randomsplit"), scoreStats);

      weakLearner = exactWeakLearner(randomSequentialNodeBuilder(numVariables, complexity));
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("random"), scoreStats);

      weakLearner = laminatingWeakLearner(randomSequentialNodeBuilder(numVariables, complexity), (double)numVariables, minExamplesForLaminating);
      testConditionLearner(context, learnerConstructor, weakLearner, str + T("laminating"), scoreStats);
    }
    context.leaveScope(new Pair(scoreStats.getMinimum(), scoreStats.getMean()));
  }

  double testConditionLearner(ExecutionContext& context, const LuapeLearnerPtr& learner, const String& name) const
  {
    context.enterScope(name);

    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics(T("error"));

    for (size_t i = 0; i < splits.size(); ++i)
    {
      context.enterScope(T("Split ") + String((int)i));
      context.resultCallback(T("split"), i);
      ScoreObjectPtr score = trainAndTest(context, learner, splits[i].first, splits[i].second);
      jassert(score);
      stats->push(score->getScoreToMinimize());
      context.leaveScope(score->getScoreToMinimize());
      context.progressCallback(new ProgressionState(i+1, splits.size(), T("Splits")));
    }

    context.leaveScope(stats);
    return stats->getMean();
  }

  ScoreObjectPtr trainAndTest(ExecutionContext& context, const LuapeLearnerPtr& learner, const ContainerPtr& trainingData, const ContainerPtr& testingData) const
  {
    LuapeClassifierPtr classifier = createClassifier(inputClass);
    if (!classifier->initialize(context, inputClass, labels))
      return ScoreObjectPtr();
    LuapeBatchLearnerPtr batchLearner = new LuapeBatchLearner(learner);
    classifier->setBatchLearner(batchLearner);
    classifier->setEvaluator(defaultSupervisedEvaluator());
    return classifier->train(context, trainingData, testingData, String::empty, false);
  }

protected:
  friend class LuapeClassificationSandBoxClass;

  File dataFile;
  size_t maxExamples;
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
    ContainerPtr res = classificationARFFDataParser(context, file, inputClass, labels, sparseData)->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
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

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CLASSIFICATION_SAND_BOX_H_
