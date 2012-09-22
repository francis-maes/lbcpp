/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearningFormulaSearchProblem.h | Learning Formula Discovery |
| Author  : Francis Maes                   |                                 |
| Started : 02/05/2012 16:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING_H_
# define LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING_H_

# include "LuapeFormulaDiscovery.h"
# include "LuapeClassificationSandBox.h"

namespace lbcpp
{

class LuapeLearningFormulaSearchProblem : public LuapeNodeSearchProblem
{
public:
  LuapeLearningFormulaSearchProblem(const File& dataFile = File(), size_t maxExamples = 0, size_t numTrainingExamples = 0, bool isMultiPass = false)
    : dataFile(dataFile), maxExamples(0), numTrainingExamples(0), isMultiPass(isMultiPass) {}

  struct Dataset
  {
    Dataset() : numFeatures(0) {}

    DefaultEnumerationPtr labels;
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

      ScalarVariableStatistics accStats;
      for (size_t iter = 0; iter < 10; ++iter)
        for (size_t i = 0; i < datasets.size(); ++i)
        {
          FunctionPtr function = libLinearClassifier(1.0, l2RegularizedLogisticRegression);
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
      context.informationCallback(T("Mean Score = ") + String(accStats.getMean()));
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

    addInput(doubleType, "p[c]");
    addInput(doubleType, "mu[f]");
    addInput(doubleType, "sigma[f]");
    addInput(doubleType, "mu[f|c]");
    addInput(doubleType, "sigma[f|c]");
    if (isMultiPass)
    {
      addInput(doubleType, "prevparam");
      addInput(doubleType, "p[correct|c]");
      addInput(doubleType, "mu[f|c,correct]");
      addInput(doubleType, "sigma[f|c,correct]");
      addInput(doubleType, "mu[f|c,error]");
      addInput(doubleType, "sigma[f|c,error]");
      addInput(doubleType, "grad[f|c]");
    }

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

    DenseDoubleVectorPtr currentParameters;
    if (isMultiPass)
      currentParameters = makeInitialParameters(datasets[0], datasets[0].data);
    samplesCache = makeInputs(context, datasets[0], datasets[0].data, currentParameters); // used in the key computation, that is only based on the first dataset
    return true;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
    {worst = 0.0; best = 1.0;}

  struct FeatureInfo
  {
    FeatureInfo(size_t numLabels)
    {
      perLabel.resize(numLabels);
      perLabelCorrect.resize(numLabels);
      perLabelError.resize(numLabels);
    }

    void observe(double value, size_t label)
    {
      overall.push(value);
      perLabel[label].push(value);
    }

    void observe(double value, size_t label, bool isCorrectlyClassified)
    {
      observe(value, label);
      if (isCorrectlyClassified)
      {
        correct.push(value);
        perLabelCorrect[label].push(value);
      }
      else
      {
        error.push(value);
        perLabelError[label].push(value);
      }
    }

    ScalarVariableStatistics overall;
    ScalarVariableStatistics correct;
    ScalarVariableStatistics error;
    std::vector<ScalarVariableStatistics> perLabel;
    std::vector<ScalarVariableStatistics> perLabelCorrect;
    std::vector<ScalarVariableStatistics> perLabelError;
  };


  DenseDoubleVectorPtr computeParameters(ExecutionContext& context, LuapeSamplesCachePtr samples, const LuapeNodePtr& node)
  {
    // compute parameters using formula
    DenseDoubleVectorPtr parameters = samples->getSamples(context, node)->getVector().staticCast<DenseDoubleVector>();
    if (parameters)
    {
      // clean missing values
      for (size_t i = 0; i < parameters->getNumValues(); ++i)
        if (parameters->getValue(i) == doubleMissingValue)
          parameters->setValue(i, 0.0);
    }
    return parameters;
  }

  virtual double computeObjective(ExecutionContext& context, const LuapeNodePtr& node, size_t instanceIndex)
  {
    // get train/test split
    Dataset& dataset = datasets[instanceIndex % datasets.size()];
    size_t splitIndex = instanceIndex / datasets.size();
    while (dataset.randomizedData.size() <= splitIndex)
      dataset.randomizedData.push_back(dataset.data->randomize());
    ContainerPtr data = dataset.randomizedData[splitIndex];
    
    size_t numTrain = this->numTrainingExamples ? this->numTrainingExamples : data->getNumElements() / 2;
    ContainerPtr trainingData = data->range(0, numTrain);
    ContainerPtr testingData = data->invRange(0, numTrain);
    
    double testAccuracy;
    if (isMultiPass)
    {
      DenseDoubleVectorPtr currentParameters = new DenseDoubleVector(dataset.labels->getNumElements() * dataset.numFeatures, 0.0);// makeInitialParameters(dataset, trainingData);
      //for (size_t i = 0; i < currentParameters->getNumValues(); ++i)
      //  currentParameters->setValue(i, context.getRandomGenerator()->sampleDoubleFromGaussian());
      for (size_t i = 0; i < 5; ++i)
      {
        LuapeSamplesCachePtr samples = makeInputs(context, dataset, trainingData, currentParameters);
        currentParameters = computeParameters(context, samples, node);
        //double trainAccuracy = evaluateAccuracy(context, dataset, trainingData, currentParameters);
        //testAccuracy = evaluateAccuracy(context, dataset, testingData, currentParameters);
        //context.informationCallback(T("i=") + String((int)i) + T(" Train: ") + String(trainAccuracy * 100, 2) + T("% Test: ") + String(testAccuracy * 100, 2) + T("%"));
      }
      testAccuracy = evaluateAccuracy(context, dataset, testingData, currentParameters);
    }
    else
    {
      LuapeSamplesCachePtr samples = makeInputs(context, dataset, trainingData);
      DenseDoubleVectorPtr parameters = computeParameters(context, samples, node);
      testAccuracy = evaluateAccuracy(context, dataset, testingData, parameters);
    }

    return testAccuracy;
  } 

  std::vector<size_t> getPredictedLabels(Dataset& dataset, const DenseDoubleVectorPtr& parameters, const DoubleVectorPtr& features) const
  {
    size_t numLabels = dataset.labels->getNumElements();
     
    std::vector<size_t> res;
    double bestScore = -DBL_MAX;
    for (size_t i = 0; i < numLabels; ++i)
    {
      double score = features->dotProduct(parameters, i * dataset.numFeatures);
      if (score >= bestScore)
      {
        if (score > bestScore)
          res.clear();
        res.push_back(i);
        bestScore = score;
      }
    }
    return res;
  }

  double evaluateAccuracy(ExecutionContext& context, Dataset& dataset, ContainerPtr examples, const DenseDoubleVectorPtr& parameters) const
  {
    RandomGeneratorPtr random = context.getRandomGenerator();

    size_t numLabels = dataset.labels->getNumElements();
    size_t n = examples->getNumElements();
    size_t numCorrect = 0;
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = examples->getElement(i).getObjectAndCast<Pair>();
      DoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>();
      size_t label = (size_t)example->getSecond().getInteger();

      std::vector<size_t> predicted = getPredictedLabels(dataset, parameters, features);
      size_t prediction = predicted.size() ?  predicted[random->sampleSize(predicted.size())] : random->sampleSize(numLabels);
      if (prediction == label)
        ++numCorrect;
    }
    return numCorrect / (double)n;
  }

  bool useVariables(const LuapeNodePtr& node) const
  {
    if (node.isInstanceOf<LuapeInputNode>())
      return true;
    size_t n = node->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
      if (useVariables(node->getSubNode(i)))
        return true;
    return false;
  }

  virtual BinaryKeyPtr makeBinaryKey(ExecutionContext& context, const LuapeNodePtr& node) const
  {
    if (!useVariables(node))
      return BinaryKeyPtr();

    DenseDoubleVectorPtr parameters = samplesCache->getSamples(context, node)->getVector().staticCast<DenseDoubleVector>();
    if (!parameters)
      return BinaryKeyPtr();

    size_t n = parameters->getNumValues();
    BinaryKeyPtr res = new BinaryKey(n * 4);
    for (size_t i = 0; i < n; ++i)
    {
      double value = parameters->getValue(i);
      if (value == doubleMissingValue)
        return BinaryKeyPtr();
      res->push32BitInteger((int)(100000 * value));
    }

/*    size_t numLabels = labels->getNumElements();
    size_t numExamples = data->getNumElements();
    BinaryKeyPtr res = new BinaryKey((numExamples * numLabels) / 8 + 1);
    for (size_t i = 0; i < numExamples; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      DoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>();
      std::vector<size_t> labels = getPredictedLabels(dataset, parameters, features);
      for (size_t j = 0; j < numLabels; ++j)
        res->pushBit(std::find(labels.begin(), labels.end(), j) != labels.end());
    }*/
    return res;
  }

protected:
  friend class LuapeLearningFormulaSearchProblemClass;

  File dataFile;
  size_t maxExamples;
  size_t numTrainingExamples;
  bool isMultiPass;

  std::vector<Dataset> datasets;
  LuapeSamplesCachePtr samplesCache;

  Dataset loadDataset(ExecutionContext& context, const File& dataFile) const
  {
    Dataset res;
    res.labels = new DefaultEnumeration("labels");
    res.data = loadData(context, dataFile, res.labels);
    if (!res.data || !res.data->getNumElements())
      return Dataset();

    res.doubleVectorClass = res.data->getElementsType()->getTemplateArgument(0);
    res.numFeatures = DoubleVector::getElementsEnumeration(res.doubleVectorClass)->getNumElements();
    context.informationCallback(String((int)res.data->getNumElements()) + T(" examples, ") +
                                String((int)res.numFeatures) + T(" variables, ") +
                                String((int)res.labels->getNumElements()) + T(" labels"));
    return res;
  }

  ContainerPtr loadData(ExecutionContext& context, const File& file, DefaultEnumerationPtr labels) const
  { 
    static const bool sparseData = true;

    context.enterScope(T("Loading ") + file.getFileName());

    ContainerPtr res;
    if (file.getFileExtension() == T(".jdb"))
    {
      DynamicClassPtr inputClass = new DynamicClass("inputs");
      TextParserPtr parser = new JDBDataParser(context, file, inputClass, labels, sparseData);
      res = parser->load(maxExamples);
      if (res)
        res = convertExamplesToVectors(res);
    }
    else if (file.getFileExtension() == T(".data"))
    {
      DefaultEnumerationPtr features = new DefaultEnumeration(T("features"));
      TextParserPtr parser = classificationLibSVMDataParser(context, file, features, labels);
      res = parser->load(maxExamples);
      /*for (size_t i = 0; i < res->getNumElements(); ++i)
      {
        PairPtr example = res->getElement(i).getObjectAndCast<Pair>();
        SparseDoubleVectorPtr features = example->getFirst().getObjectAndCast<SparseDoubleVector>();
        std::vector<std::pair<size_t, double> >& values = features->getValuesVector();
        for (size_t j = 0; j < values.size(); ++j)
          values[j].second = values[j].second * 2 - 1; // TMP! bring into [0,1]
      }*/
    }

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

  DenseDoubleVectorPtr makeInitialParameters(Dataset& dataset, ContainerPtr data) const
  {
    size_t numLabels = dataset.labels->getNumElements();
    std::vector<FeatureInfo> featureInfos(dataset.numFeatures, FeatureInfo(numLabels));
    size_t n = data->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      DenseDoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      size_t label = (size_t)example->getSecond().getInteger();
      jassert(features->getNumValues() == dataset.numFeatures);
      for (size_t j = 0; j < dataset.numFeatures; ++j)
        featureInfos[j].observe(features->getValue(j), label);
    }

    DenseDoubleVectorPtr res = new DenseDoubleVector(numLabels * dataset.numFeatures, 0.0);
    size_t index = 0;
    for (size_t i = 0; i < numLabels; ++i)
    {
      for (size_t j = 0; j < dataset.numFeatures; ++j)
      {
        ScalarVariableStatistics& stats = featureInfos[j].perLabel[i];
        res->setValue(index++, stats.getMean() - stats.getStandardDeviation());
      }
    }
    return res;
  }
  
  LuapeSamplesCachePtr makeInputs(ExecutionContext& context, Dataset& dataset, ContainerPtr data, DenseDoubleVectorPtr currentParameters = DenseDoubleVectorPtr()) const
  {
    jassert(!isMultiPass || currentParameters);

    size_t numLabels = dataset.labels->getNumElements();
    
    LuapeSamplesCachePtr res = createCache(numLabels * dataset.numFeatures, 100);

    // compute statistics
    std::vector<FeatureInfo> featureInfos(dataset.numFeatures, FeatureInfo(numLabels));
    size_t numExamples = data->getNumElements();
    DenseDoubleVectorPtr classFrequencies = new DenseDoubleVector(dataset.labels, probabilityType);
    DenseDoubleVectorPtr gradient = new DenseDoubleVector(numLabels * dataset.numFeatures, 0.0);
    for (size_t i = 0; i < numExamples; ++i)
    {
      PairPtr example = data->getElement(i).getObjectAndCast<Pair>();
      DenseDoubleVectorPtr features = example->getFirst().getObjectAndCast<DoubleVector>()->toDenseDoubleVector();
      size_t label = (size_t)example->getSecond().getInteger();

      classFrequencies->incrementValue(label, 1.0);
      jassert(features->getNumValues() == dataset.numFeatures);

      bool correctlyClassified = false;
      if (isMultiPass)
      {
        jassert(currentParameters);
        std::vector<size_t> predictions = getPredictedLabels(dataset, currentParameters, features);
        size_t prediction = 0;
        if (predictions.empty())
          correctlyClassified = false;
        else
        {
          prediction = predictions[context.getRandomGenerator()->sampleSize(predictions.size())];
          correctlyClassified = (label == prediction);
        }

        if (!correctlyClassified)
        {
          features->addWeightedTo(gradient, label * dataset.numFeatures, 1.0 / numExamples);
          features->addWeightedTo(gradient, prediction * dataset.numFeatures, -1.0 / numExamples);
        }
        //(prediction.size() == 1 && *prediction.begin() == label);
      }

      for (size_t j = 0; j < dataset.numFeatures; ++j)
      {
        if (isMultiPass)
          featureInfos[j].observe(features->getValue(j), label, correctlyClassified);
        else
          featureInfos[j].observe(features->getValue(j), label);
      }
    }
    classFrequencies->multiplyByScalar(1.0 / (double)numExamples);

    // create prediction problems
    size_t index = 0;
    for (size_t i = 0; i < numLabels; ++i)
    {
      double classFrequency = classFrequencies->getValue(i);
      for (size_t j = 0; j < dataset.numFeatures; ++j)
      {
        FeatureInfo& info = featureInfos[j];
        DenseDoubleVectorPtr variables = new DenseDoubleVector(getNumInputs(), 0.0);
        variables->setValue(0, classFrequency);
        variables->setValue(1, info.overall.getMean());
        variables->setValue(2, info.overall.getStandardDeviation());
        variables->setValue(3, info.perLabel[i].getMean());
        variables->setValue(4, info.perLabel[i].getStandardDeviation());
        if (isMultiPass)
        {
          variables->setValue(5, currentParameters->getValue(index)); // current parameter
          variables->setValue(6, info.perLabelCorrect[i].getCount() / data->getNumElements());
          variables->setValue(7, info.perLabelCorrect[i].getMean());
          variables->setValue(8, info.perLabelCorrect[i].getStandardDeviation());
          variables->setValue(9, info.perLabelError[i].getMean());
          variables->setValue(10, info.perLabelError[i].getStandardDeviation());
          variables->setValue(11, gradient->getValue(index));
        }
        res->setInputObject(inputs, index++, variables);
      }
    }
    return res;
  }
};

class LearningFormulaDiscoverySandBox : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    LuapeNodeSearchProblemPtr problem = new LuapeLearningFormulaSearchProblem(context.getFile("libsvmdata"), 0, 0, true);
    if (!problem->initializeProblem(context))
      return false;

    LuapeUniversePtr universe = problem->getUniverse();
/*
    for (size_t i = 0; i < problem->getNumInputs(); ++i)
    {
      LuapeNodePtr node = problem->getInput(i);
      context.informationCallback(T("Score of ") + node->toShortString() + T(": ") + String(evaluate(context, problem, node)));
    }*/

    context.enterScope(T("Curve"));
    for (double C = -5.0; C <= 5.0; C += 0.2)
    {
      context.enterScope(T("C = ") + String(C));
      LuapeNodePtr node = new LuapeFunctionNode(addDoubleLuapeFunction(), problem->getInput(5), 
        new LuapeFunctionNode(mulDoubleLuapeFunction(), new LuapeConstantNode(pow(10.0, C)), problem->getInput(11)));
        //new LuapeFunctionNode(mulDoubleLuapeFunction(), problem->getInput(9), 
        //  new LuapeFunctionNode(subDoubleLuapeFunction(), new LuapeConstantNode(1.0), problem->getInput(6)))));
      context.resultCallback(T("C"), C);
      context.resultCallback(T("score"), evaluate(context, problem, node));
/*
      for (double P = 1.0 / 32.0; P <= 1.0; P *= 2.0)
      {
        LuapeNodePtr node = new LuapeFunctionNode(addDoubleLuapeFunction(), problem->getInput(3), 
          new LuapeFunctionNode(mulDoubleLuapeFunction(), new LuapeConstantNode(C / P), 
            new LuapeFunctionNode(powDoubleLuapeFunction(), problem->getInput(4), new LuapeConstantNode(P))));
        context.resultCallback(T("scoreP=") + String(P), evaluate(context, problem, node));
      }
*/
      context.leaveScope();
    }
    return true;
  }

  double evaluate(ExecutionContext& context, LuapeNodeSearchProblemPtr problem, const LuapeNodePtr& node)
  {
    static const size_t precision = 100;
    double res = 0.0;
    for (size_t i = 0; i < precision; ++i)
      res += problem->computeObjective(context, node, i);
    return res / precision;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_FORMULA_DISCOVERY_LEARNING_H_
