/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearningFormulaSearchProblem2.h | Learning Formula Discovery|
| Author  : Francis Maes                   |                                 |
| Started : 02/05/2012 16:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING2_H_
# define LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING2_H_

# include "LuapeFormulaDiscovery.h"
# include "LuapeClassificationSandBox.h"
# include "EmpiricalContinuousDistribution.h"

namespace lbcpp
{

class BinaryClassificationFormulaSearchProblem : public LuapeNodeSearchProblem
{
public:
  BinaryClassificationFormulaSearchProblem(const File& dataFile = File())
    : dataFile(dataFile) {}

  virtual ClassPtr getFeatureInformationClass() const = 0;
  virtual void makeFeatureInformations(const ContainerPtr& data, size_t numFeatures, std::vector<ObjectPtr>& res) const = 0;
  virtual void createActiveVariables(ExecutionContext& context) = 0;
 
  struct Dataset
  {
    Dataset() : numFeatures(0) {}

    ContainerPtr data;
    size_t numFeatures;
    ClassPtr doubleVectorClass;
    std::vector<ContainerPtr> randomizedData;
  };

  virtual bool initializeProblem(ExecutionContext& context)
  {
    if (dataFile.isDirectory())
    {
      context.enterScope(T("Loading datasets in ") + dataFile.getFileName());
      juce::OwnedArray<File> files;
      dataFile.findChildFiles(files, File::findFiles, false, "*.*");
      for (int i = 0; i < files.size(); ++i)
      {
        Dataset dataset = loadDataset(context, *files[i]);
        if (dataset.data)
          datasets.push_back(dataset);
      }
      context.informationCallback(String((int)datasets.size()) + T(" datasets"));
      context.leaveScope(datasets.size());

     /* ScalarVariableStatistics accStats;
      for (size_t iter = 0; iter < 10; ++iter)
        for (size_t i = 0; i < datasets.size(); ++i)
        {
          FunctionPtr function = libLinearBinaryClassifier(1.0, l2RegularizedLogisticRegression);
          ContainerPtr data = datasets[i].data->randomize();
          function->train(context, data->fold(0, 2));
          ScoreObjectPtr score = function->evaluate(context, data->invFold(0, 2), classificationEvaluator());
          if (score)
          {
            double accuracy = 1.0 - score->getScoreToMinimize();
            accStats.push(accuracy);
            context.informationCallback(T("Dataset ") + String((int)i) + T(" score = ") + String(accuracy));
          }
        }
      context.informationCallback(T("Mean Score = ") + String(accStats.getMean()));*/
    }
    else
    {
      Dataset dataset = loadDataset(context, dataFile);
      if (!dataset.data)
        return false;
      datasets.push_back(dataset);
    }

    // setup search space
    addConstant(0.001);
    addConstant(0.01);
    addConstant(0.1);

    addConstant(1.0);
    addConstant(2.0);
    addConstant(3.0);
    addConstant(5.0);
    addConstant(7.0);

    addInput(getFeatureInformationClass(), "i");
    addInput(doubleType, "f");
    addInput(doubleType, "D"); // dimensionality

    addFunction(getVariableLuapeFunction());

    addFunction(oppositeDoubleLuapeFunction());
    addFunction(inverseDoubleLuapeFunction());
    addFunction(sqrtDoubleLuapeFunction());
    addFunction(logDoubleLuapeFunction());
    addFunction(absDoubleLuapeFunction());

    addFunction(addDoubleLuapeFunction());
    addFunction(subDoubleLuapeFunction());
    addFunction(mulDoubleLuapeFunction());
    addFunction(divDoubleLuapeFunction());
    addFunction(minDoubleLuapeFunction());
    addFunction(maxDoubleLuapeFunction());

    addTargetType(doubleType);

    createActiveVariables(context);

    samplesCache = makeSamples(context, datasets[0], datasets[0].data->fold(0, 2), datasets[0].data->invFold(0, 2)); // used in the key computation, that is only based on the first dataset
    return true;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  virtual double computeObjective(ExecutionContext& context, const LuapeNodePtr& node, size_t instanceIndex)
  {
    // get train/test split
    Dataset& dataset = datasets[instanceIndex % datasets.size()];
    size_t splitIndex = instanceIndex / datasets.size();
    while (dataset.randomizedData.size() <= splitIndex)
      dataset.randomizedData.push_back(dataset.data->randomize());
    ContainerPtr data = dataset.randomizedData[splitIndex];
    ContainerPtr trainingData = data->fold(0, 2);
    ContainerPtr testingData = data->invFold(0, 2);
    //double percentageOfPositives = getPercentageOfPositives(trainingData);
    //context.informationCallback(T("PP: ") + String(percentageOfPositives));

    // make samples
    LuapeSamplesCachePtr samples = makeSamples(context, dataset, trainingData, testingData);
    DenseDoubleVectorPtr activations = samples->getSamples(context, node)->getVector().staticCast<DenseDoubleVector>();
    if (!activations)
      return 0.0;

    // evaluate accuracy 
    size_t numTestExamples = testingData->getNumElements();
    size_t numCorrect = 0;
    size_t index = 0;
    for (size_t i = 0; i < numTestExamples; ++i)
    {
      PairPtr example = testingData->getElement(i).getObjectAndCast<Pair>();
      DenseDoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      bool supervision = example->getSecond().getBoolean();
      
      double score = 0.0;
      for (size_t j = 0; j < dataset.numFeatures; ++j)
        score += activations->getValue(index++);
      bool prediction = (score > 0);

      if (supervision == prediction)
        ++numCorrect;
    }
    return numCorrect / (double)numTestExamples;
  } 

  bool useNode(const LuapeNodePtr& node, const LuapeNodePtr& subNode) const
  {
    if (node == subNode)
      return true;
    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      if (useNode(node->getSubNode(i), subNode))
        return true;
    return false;
  }

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const LuapeNodePtr& node) const
  {
    if (!useNode(node, getInput(0)) || !useNode(node, getInput(1)))
      return BinaryKeyPtr();

    DenseDoubleVectorPtr activations = samplesCache->getSamples(context, node)->getVector().staticCast<DenseDoubleVector>();
    if (!activations)
      return BinaryKeyPtr();

    size_t numFeatures = datasets[0].numFeatures;
    size_t numExamples = activations->getNumValues() / numFeatures;

    BinaryKeyPtr res = new BinaryKey((numExamples + 7) / 8);
    size_t index = 0;
    for (size_t i = 0; i < numExamples; ++i)
    {
      double score = 0.0;
      for (size_t j = 0; j < numFeatures; ++j)
      {
        double activation = activations->getValue(index++);
        if (activation == doubleMissingValue)
          return BinaryKeyPtr();
        score += activation;
      }
      res->pushBit(score > 0);
    }
    return res;
  }

protected:
  friend class BinaryClassificationFormulaSearchProblemClass;

  File dataFile;
  size_t numExamples;
  std::vector<Dataset> datasets;
  LuapeSamplesCachePtr samplesCache;

  Dataset loadDataset(ExecutionContext& context, const File& dataFile) const
  {
    Dataset res;
    res.data = loadData(context, dataFile);
    if (!res.data || !res.data->getNumElements())
      return Dataset();

    res.doubleVectorClass = res.data->getElementsType()->getTemplateArgument(0);
    res.numFeatures = DoubleVector::getElementsEnumeration(res.doubleVectorClass)->getNumElements();
    context.informationCallback(String((int)res.data->getNumElements()) + T(" examples, ") +
                                String((int)res.numFeatures) + T(" variables"));
    return res;
  }

  ContainerPtr loadData(ExecutionContext& context, const File& file) const
  { 
    static const bool sparseData = true;

    context.enterScope(T("Loading ") + file.getFileName());

    DefaultEnumerationPtr features = new DefaultEnumeration(T("features"));
    TextParserPtr parser = binaryClassificationLibSVMDataParser(context, file, features);
    ContainerPtr res = parser->load(numExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();

    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  LuapeSamplesCachePtr makeSamples(ExecutionContext& context, Dataset& dataset, const ContainerPtr& trainingData, const ContainerPtr& testingData)
  {
    // make feature informations based on training data
    std::vector<ObjectPtr> featureInformations;
    makeFeatureInformations(trainingData, dataset.numFeatures, featureInformations);

    // make samples cache based on testing data
    size_t numTestExamples = testingData->getNumElements();
    LuapeSamplesCachePtr res = createCache(numTestExamples * dataset.numFeatures, 100);
    ClassPtr exampleType = pairClass(getFeatureInformationClass(), doubleType);
    size_t index = 0;
    for (size_t i = 0; i < numTestExamples; ++i)
    {
      PairPtr example = testingData->getElement(i).getObjectAndCast<Pair>();
      DenseDoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      for (size_t j = 0; j < dataset.numFeatures; ++j)
      {
        VariableVectorPtr input = new VariableVector(3);
        input->setElement(0, featureInformations[j]);
        input->setElement(1, features->getValue(j));
        input->setElement(2, (double)dataset.numFeatures);
        res->setInputObject(inputs, index++, input);
      }
    }
    return res;
  }

  double getPercentageOfPositives(const ContainerPtr& data) const
  {
    size_t n = data->getNumElements();
    size_t numPositives = 0;
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      bool isPositive = example->getSecond().getBoolean();
      if (isPositive)
        ++numPositives;
    }
    return numPositives / (double)n;
  }
};

////////////////////////////////

class SimpleContinuousFeatureInformation : public Object
{
public:
  SimpleContinuousFeatureInformation() :
    all(new EmpiricalContinuousDistribution()),
    pos(new EmpiricalContinuousDistribution()),
    neg(new EmpiricalContinuousDistribution()) {}

  void observe(double value, bool isPositive)
  {
    all->observe(value);
    if (isPositive)
      pos->observe(value);
    else
      neg->observe(value);
  }

  EmpiricalContinuousDistributionPtr all;
  EmpiricalContinuousDistributionPtr pos;
  EmpiricalContinuousDistributionPtr neg;
};

typedef ReferenceCountedObjectPtr<SimpleContinuousFeatureInformation> SimpleContinuousFeatureInformationPtr;

extern ClassPtr simpleContinuousFeatureInformationClass;

extern ClassPtr getVariableLuapeFunctionClass;
extern ClassPtr subDoubleLuapeFunctionClass;
extern ClassPtr divDoubleLuapeFunctionClass;

class SimpleContinuousFeatureBCFSP : public BinaryClassificationFormulaSearchProblem
{
public:
  virtual ClassPtr getFeatureInformationClass() const
    {return simpleContinuousFeatureInformationClass;}

  virtual void makeFeatureInformations(const ContainerPtr& data, size_t numFeatures, std::vector<ObjectPtr>& res) const
  {
    res.resize(numFeatures);
    size_t numExamples = data->getNumElements();
    res.resize(numFeatures);
    for (size_t i = 0; i < numFeatures; ++i)
      res[i] = new SimpleContinuousFeatureInformation();

    for (size_t i = 0; i < numExamples; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      DenseDoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      bool isPositive = example->getSecond().getBoolean();

      for (size_t j = 0; j < numFeatures; ++j)
        res[j].staticCast<SimpleContinuousFeatureInformation>()->observe(features->getValue(j), isPositive);
    }
  }

  virtual void createActiveVariables(ExecutionContext& context)
  {
    std::vector<Variable> v(2);
    v[0] = simpleContinuousFeatureInformationClass;
    v[1] = Variable(0, positiveIntegerType);
    LuapeNodePtr all = universe->makeFunctionNode(universe->makeFunction(getVariableLuapeFunctionClass, v), getInput(0));
    v[1] = Variable(1, positiveIntegerType);
    LuapeNodePtr pos = universe->makeFunctionNode(universe->makeFunction(getVariableLuapeFunctionClass, v), getInput(0));
    v[1] = Variable(2, positiveIntegerType);
    LuapeNodePtr neg = universe->makeFunctionNode(universe->makeFunction(getVariableLuapeFunctionClass, v), getInput(0));

    addActiveVariable(all);
    addActiveVariable(pos);
    addActiveVariable(neg);

    addFunction(new EmpiricalContinuousDistributionNumSamplesFunction());
    addFunction(new EmpiricalContinuousDistributionMeanFunction());
    addFunction(new EmpiricalContinuousDistributionSquaresMeanFunction());
    addFunction(new EmpiricalContinuousDistributionVarianceFunction());
    addFunction(new EmpiricalContinuousDistributionStandardDeviationFunction());
    addFunction(new EmpiricalContinuousDistributionMinimumFunction());
    addFunction(new EmpiricalContinuousDistributionMaximumFunction());
    addFunction(new EmpiricalContinuousDistributionMedianFunction());
   
    v.clear();
    LuapeNodePtr mu = universe->makeFunctionNode(universe->makeFunction(empiricalContinuousDistributionMeanFunctionClass, v), all);
    LuapeNodePtr stddev = universe->makeFunctionNode(universe->makeFunction(empiricalContinuousDistributionStandardDeviationFunctionClass, v), all);
    LuapeNodePtr muPos = universe->makeFunctionNode(universe->makeFunction(empiricalContinuousDistributionMeanFunctionClass, v), pos);
    LuapeNodePtr muNeg = universe->makeFunctionNode(universe->makeFunction(empiricalContinuousDistributionMeanFunctionClass, v), neg);

    addActiveVariable(mu);
    addActiveVariable(muPos);
    addActiveVariable(muNeg);

    addActiveVariable(universe->makeFunctionNode(
      universe->makeFunction(subDoubleLuapeFunctionClass, std::vector<Variable>()),
      muPos, muNeg)); // i.muPos - i.muNeg
    addActiveVariable(universe->makeFunctionNode(
      universe->makeFunction(subDoubleLuapeFunctionClass, std::vector<Variable>()),
      muPos, mu));  // i.muPos - i.mu
    LuapeNodePtr fMinusMu = universe->makeFunctionNode(
      universe->makeFunction(subDoubleLuapeFunctionClass, std::vector<Variable>()),
      getInput(1), mu);
    addActiveVariable(fMinusMu); // f - i.mu
    addActiveVariable(universe->makeFunctionNode(
      universe->makeFunction(divDoubleLuapeFunctionClass, std::vector<Variable>()),
      fMinusMu, stddev)); // (f - i.mu) / i.stddev*/
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING2_H_
