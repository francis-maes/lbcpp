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
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{

# ifdef JUCE_WIN32
#  pragma warning(disable:4996) // microsoft visual does not like fopen()/fclose()
# endif // JUCE_WIN32

class TestingSetParser : public TextParser
{
public:
  TestingSetParser(ExecutionContext& context, const juce::File& file, ContainerPtr data)
    : TextParser(context, file), data(data) {}
  TestingSetParser() {}

  virtual ClassPtr getElementsType() const
  {
    ClassPtr exampleType = data->getElementsType();
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
        context.warningCallback(T("Invalid index ") + string(token) + T(" (num examples = ") + string((int)data->getNumElements()) + T(")"));
        //return false;
        continue;
      }
      size_t idx = (size_t)(index - 1);
      if (testingIndices.find(idx) != testingIndices.end())
        context.warningCallback(T("Redundant index ") + string(token));
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

    ClassPtr exampleType = data->getElementsType();
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
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    std::vector<VariableExpressionPtr> inputs;
    VariableExpressionPtr supervision;
    TablePtr dataset = loadDataFile(context, dataFile, inputs, supervision);
    if (!dataset || !supervision)
      return new Boolean(false);

    size_t numVariables = inputs.size();
    size_t numExamples = dataset->getNumRows();
    size_t numLabels = supervision->getType().staticCast<Enumeration>()->getNumElements();

    context.informationCallback(string((int)numExamples) + T(" examples, ") +
                                string((int)numVariables) + T(" variables, ") +
                                string((int)numLabels) + T(" labels"));

    context.resultCallback("dataset", dataset);
    context.resultCallback("supervision", supervision);

    /* make splits
    File tsFile = dataFile.withFileExtension("txt");
    std::vector< std::pair< ContainerPtr, ContainerPtr > > splits;
    context.enterScope(T("Splits"));
    if (makeSplits(context, tsFile, inputClass, data, splits) && verbosity > 0)
    {
      for (size_t i = 0; i < splits.size(); ++i)
        context.informationCallback(T("Split ") + string((int)i) + T(": train size = ") + string((int)splits[i].first->getNumElements())
                              + T(", test size = ") + string((int)splits[i].second->getNumElements()));
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
    ClassPtr inputDoubleVectorType = trainingData->getElementsType()->getTemplateArgument(0);*/

    // todo:

    return new Boolean(true);
  }
  
private:
  friend class ClassificationSandBoxClass;

  juce::File dataFile;
  size_t maxExamples;
  size_t verbosity;

  TablePtr loadDataFile(ExecutionContext& context, const juce::File& file, std::vector<VariableExpressionPtr>& inputs, VariableExpressionPtr& supervision)
  {
    context.enterScope(T("Loading ") + file.getFileName());
    TablePtr res = Object::createFromFile(context, file);
    context.leaveScope(res ? res->getNumRows() : 0);
    // FIXME: bind table with variables
    // FIXME: inputs, supervision
    return res;
  }

  /*
  bool makeSplits(ExecutionContext& context, const juce::File& tsFile, DynamicClassPtr inputClass, ContainerPtr data, std::vector< std::pair< ContainerPtr, ContainerPtr > >& res)
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
    
    ClassPtr dvClass = denseDoubleVectorClass(variablesEnumerationEnumeration(p->getFirst().getType()), doubleClass);
    ClassPtr supType = p->getSecond().getType();
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
      res->setValue(i, example->getVariable(i)->toDouble());
    return res;
  }*/
};

}; /* namespace lbcpp */

#endif // LBCPP_MCGP_CLASSIFICATION_SANDBOX_H_
