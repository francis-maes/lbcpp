/*-----------------------------------------.---------------------------------.
| Filename: ClassificationSandBox.h        | Classification SandBox          |
| Author  : Francis Maes                   |                                 |
| Started : 09/11/2012 10:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
# define LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/Stream.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Luape/LuapeCache.h>
# include <lbcpp-ml/Expression.h>

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

class ClassificationSandBox : public WorkUnit
{
public:
  ClassificationSandBox() : maxExamples(0), verbosity(0) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    VariableExpressionPtr supervision;
    LuapeSamplesCachePtr dataset = loadDataFile(context, dataFile, supervision);
    if (!dataset || !supervision)
      return false;

    size_t numVariables = dataset->getNumberOfCachedNodes() - 1;
    size_t numExamples = dataset->getNumSamples();
    size_t numLabels = supervision->getType().staticCast<Enumeration>()->getNumElements();

    context.informationCallback(String((int)numExamples) + T(" examples, ") +
                                String((int)numVariables) + T(" variables, ") +
                                String((int)numLabels) + T(" labels"));

    context.resultCallback("dataset", dataset);
    context.resultCallback("supervision", supervision);

    /* make splits
    File tsFile = dataFile.withFileExtension("txt");
    std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;
    context.enterScope(T("Splits"));
    if (makeSplits(context, tsFile, inputClass, data, splits) && verbosity > 0)
    {
      for (size_t i = 0; i < splits.size(); ++i)
        context.informationCallback(T("Split ") + String((int)i) + T(": train size = ") + String((int)splits[i].first->getNumElements())
                              + T(", test size = ") + String((int)splits[i].second->getNumElements()));
    }
    context.leaveScope(splits.size());
    if (!splits.size())
      return false;*/
   /* if (foldNumber >= splits.size())
    {
      context.errorCallback(T("Invalid fold number"));
      return false;
    }
    ContainerPtr trainingData = splits[foldNumber].first;
    ContainerPtr testingData = splits[foldNumber].second;
    TypePtr inputDoubleVectorType = trainingData->getElementsType()->getTemplateArgument(0);*/

    // todo:

    return true;
  }
  
private:
  friend class ClassificationSandBoxClass;

  File dataFile;
  size_t maxExamples;
  size_t verbosity;

  LuapeSamplesCachePtr loadDataFile(ExecutionContext& context, const File& file, VariableExpressionPtr& supervision)
  {
    LuapeSamplesCachePtr res;

    context.enterScope(T("Loading ") + file.getFileName());
    static const bool sparseData = true;
    
    DynamicClassPtr inputClass = new DynamicClass("inputs");
    DefaultEnumerationPtr labels = new DefaultEnumeration("labels");

    TextParserPtr parser;
    if (file.getFileExtension() == T(".jdb"))
      parser = new JDBDataParser(context, file, inputClass, labels, sparseData);
    else
      parser = classificationARFFDataParser(context, file, inputClass, labels, sparseData);
    ContainerPtr container = parser->load(maxExamples);
    if (container && container->getNumElements())
    {
      PairPtr p = container->getElement(0).getObjectAndCast<Pair>();
      
      TypePtr inputType = p->getFirst().getType();
      TypePtr outputType = p->getSecond().getType();

      std::vector<VariableExpressionPtr> inputs(inputType->getNumMemberVariables());
      for (size_t i = 0; i < inputs.size(); ++i)
        inputs[i] = new VariableExpression(inputType->getMemberVariableType(i), inputType->getMemberVariableName(i), i);
      supervision = new VariableExpression(outputType, "supervision", inputs.size());

      res = new LuapeSamplesCache(ExpressionUniversePtr(), inputs, container->getNumElements());
      VectorPtr supervisionValues = vector(outputType, container->getNumElements());
      for (size_t i = 0; i < container->getNumElements(); ++i)
      {
        PairPtr p = container->getElement(i).getObjectAndCast<Pair>();
        res->setInputObject(inputs, i, p->getFirst().getObject());
        supervisionValues->setElement(i, p->getSecond());
      }
      res->cacheNode(context, supervision, supervisionValues, "Supervision", false);
    }

    context.leaveScope(res ? res->getNumSamples() : 0);
    return res;
  }

  /*
  bool makeSplits(ExecutionContext& context, const File& tsFile, DynamicClassPtr inputClass, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
  {
    if (tsFile.existsAsFile())
    {
      ContainerPtr convertedData = convertExamplesToVectors(data.staticCast<ObjectVector>());

      TextParserPtr parser = new TestingSetParser(context, tsFile, convertedData);
      ContainerPtr splits = parser->load(0);
      res.resize(splits->getNumElements());
      for (size_t i = 0; i < res.size(); ++i)
      {
        PairPtr split = splits->getElement(i).getObjectAndCast<Pair>();
        ContainerPtr train = split->getFirst().getObjectAndCast<Container>();
        ContainerPtr test = split->getSecond().getObjectAndCast<Container>();
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

  static DenseDoubleVectorPtr convertExampleToVector(const ObjectPtr& example, const ClassPtr& dvClass)
  {
    DenseDoubleVectorPtr res = new DenseDoubleVector(dvClass);
    size_t n = res->getNumValues();
    jassert(n == example->getNumVariables());
    for (size_t i = 0; i < n; ++i)
      res->setValue(i, example->getVariable(i).toDouble());
    return res;
  }*/
};

}; /* namespace lbcpp */

#endif // LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
