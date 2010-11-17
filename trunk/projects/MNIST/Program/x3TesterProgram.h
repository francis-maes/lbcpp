#ifndef LBCPP_MNIST_x3_H_
# define LBCPP_MNIST_x3_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

extern EnumerationPtr waveFormTypeEnumeration;
  
class FlattenContainerPerception : public Perception
{
public:
  FlattenContainerPerception(size_t numElements = 10, TypePtr elementType = doubleType)
  : numElements(numElements), elementType(elementType) {}

  virtual TypePtr getInputType() const
    {return vectorClass(elementType);}
  
  virtual void computeOutputType()
  {
    for (size_t i = 0; i < numElements; ++i)
      addOutputVariable(T("FlatContPer[") + String((int)i) + T("]"), elementType);
    Perception::computeOutputType();
  }
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr data = input.getObjectAndCast<Container>();
    for (size_t i = 0; i < numElements; ++i)
      callback->sense(i, data->getElement(i).getDouble());
  }
  
protected:
  friend class FlattenContainerPerceptionClass;
  
  size_t numElements;
  TypePtr elementType;
};
  
class X3TesterProgram : public Program
{
public:
  X3TesterProgram() : numTrees(100), numAttributes(10), minSplitSize(1) {}
  
  virtual String toString() const
    {return T("x3Tester has one goal in live: Make Extra-Trees really works ;-)");}
  
  virtual int runProgram(MessageCallback& callback)
  {
    File input(T("/Users/jbecker/Documents/Workspace/Data/x3TestData/waveform.txt"));
    
    std::vector<std::vector<double> > data;
    parseDataFile(input, data);
    
    ContainerPtr learningData = loadDataToContainer(data, 0, 300);
    ContainerPtr testingData = loadDataToContainer(data, 3000, 5000);

    InferenceContextPtr context = singleThreadedInferenceContext();
    PerceptionPtr perception = flattenPerception(new FlattenContainerPerception(21));
    InferencePtr inference = classificationExtraTreeInference(T("x3Test"), perception, waveFormTypeEnumeration, numTrees, numAttributes, minSplitSize);
    
    context->train(inference, learningData, ContainerPtr());
    
    EvaluatorPtr evaluator = classificationAccuracyEvaluator(T("x3TestEvaluator"));
    context->evaluate(inference, learningData, evaluator);
    std::cout << "Evaluation (Train)" << std::endl << evaluator->toString() << std::endl;
    
    evaluator = classificationAccuracyEvaluator(T("x3TestEvaluator"));
    context->evaluate(inference, testingData, evaluator);
    std::cout << "Evaluation (Test)" << std::endl << evaluator->toString() << std::endl;
    
    return 0;
  }

protected:
  friend class X3TesterProgramClass;
  
  size_t numTrees;
  size_t numAttributes;
  size_t minSplitSize;

private:
  void parseDataFile(const File& file, std::vector<std::vector<double> >& results)
  {
    InputStream* is = file.createInputStream();
    while (!is->isExhausted())
    {
      std::vector<double> example;
      StringArray tokens;
      tokens.addTokens(is->readNextLine(), T(" "));
      for (size_t i = 0; i < (size_t)tokens.size(); ++i)
        example.push_back(tokens[i].getDoubleValue());
      jassert(example.size() == 22);
      results.push_back(example);
    }
    jassert(results.size() == 5000);
    delete is;
  }
  
  ContainerPtr loadDataToContainer(const std::vector<std::vector<double> >& data, size_t from, size_t to)
  {
    jassert(from <= to && to <= data.size());
    ContainerPtr res = vector(pairClass(vectorClass(doubleType), waveFormTypeEnumeration), to - from);
    for (size_t i = from; i < to; ++i)
    {
      ContainerPtr example = vector(doubleType, data[i].size());
      for (size_t j = 0; j < data[i].size(); ++j)
        example->setElement(j, Variable(data[i][j], doubleType));
      res->setElement(i - from, Variable::pair(example, Variable((int)data[i][data[i].size() - 1], waveFormTypeEnumeration)));
    }
    return res;
  }
};

};

#endif // !LBCPP_MNIST_x3_H_
