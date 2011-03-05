/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeTestUnit.h            | ExtraTree Test Unit             |
| Author  : Julien Becker                  |                                 |
| Started : 16/12/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_TEST_UNIT_EXTRA_TREES_H_
# define LBCPP_TEST_UNIT_EXTRA_TREES_H_

# if 0 // broken

# include <lbcpp/Execution/TestUnit.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/DecisionTree/DecisionTree.h>
# include <lbcpp/Function/OldEvaluator.h>

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
  
  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    ContainerPtr data = input.getObjectAndCast<Container>(context);
    for (size_t i = 0; i < numElements; ++i)
      callback->sense(i, data->getElement(i).getDouble());
  }
  
protected:
  friend class FlattenContainerPerceptionClass;
  
  size_t numElements;
  TypePtr elementType;
};
  
class ExtraTreeTestUnit : public TestUnit
{
public:
  ExtraTreeTestUnit() : numTrees(100), numAttributes(21), minSplitSize(1) {}
  
  virtual String toString() const
    {return T("x3Tester has one goal in live: Make Extra-Trees really works ;-)");}
  
  virtual Variable run(ExecutionContext& context)
  {
    runClassification(context);
    runRegression(context);
    return true;
  }

protected:
  friend class ExtraTreeTestUnitClass;
  
  size_t numTrees;
  size_t numAttributes;
  size_t minSplitSize;

  void runClassification(ExecutionContext& context)
  {  
    //File input(File::getSpecialLocation(File::currentExecutableFile).getChildFile(T("../../projects/Examples/Data/ExtraTrees/classification.csv")));
    File input(File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/ExtraTrees/classification.csv")));

    std::vector<std::vector<double> > data;
    parseDataFile(context, input, data, true);
    
    ContainerPtr learningData = loadDataToContainer(data, 0, 300, true);
    ContainerPtr testingData = loadDataToContainer(data, 300, 2300, true);
    
    PerceptionPtr perception = flattenPerception(new FlattenContainerPerception(21));
    InferencePtr inference = classificationExtraTreeInference(T("x3Test"), perception, waveFormTypeEnumeration, numTrees, numAttributes, minSplitSize);

    inference->train(context, learningData, ContainerPtr());
    jassertfalse;
    OldEvaluatorPtr evaluator = OldEvaluatorPtr(); // FIXME
    
    inference->evaluate(context, learningData, evaluator, T("Evaluating on training data"));
    checkIsCloseTo(context, 1.0, 0.0, evaluator->getDefaultScore());
    jassertfalse;
    evaluator = OldEvaluatorPtr(); //FIXME
    inference->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));
    checkIsCloseTo(context, 0.85, 0.03, evaluator->getDefaultScore());
  }
  
  void runRegression(ExecutionContext& context)
  {
    //File input(File::getSpecialLocation(File::currentExecutableFile).getChildFile(T("../../../projects/Examples/Data/ExtraTrees/regression.csv")));
    File input(File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/ExtraTrees/regression.csv")));
    
    std::vector<std::vector<double> > data;
    parseDataFile(context, input, data, false);
    
    ContainerPtr learningData = loadDataToContainer(data, 0, 300, false);
    ContainerPtr testingData = loadDataToContainer(data, 300, 2300, false);
    
    PerceptionPtr perception = flattenPerception(new FlattenContainerPerception(10));
    InferencePtr inference = regressionExtraTreeInference(T("x3Test"), perception, numTrees, numAttributes, minSplitSize);

    inference->train(context, learningData, ContainerPtr());
    jassertfalse;
    OldEvaluatorPtr evaluator = OldEvaluatorPtr(); //FIXME
    inference->evaluate(context, learningData, evaluator);
    checkIsCloseTo(context, 0.0, 0.0001, evaluator->getDefaultScore());
    jassertfalse;
    evaluator = OldEvaluatorPtr(); // FIXME
    inference->evaluate(context, testingData, evaluator);
    checkIsCloseTo(context, 2.2, 0.3, -evaluator->getDefaultScore());
  }

private:  
  void parseDataFile(ExecutionContext& context, const File& file, std::vector<std::vector<double> >& results, bool isClassification)
  {
    InputStream* is = file.createInputStream();
    if (!is)
    {
      context.errorCallback(T("Could not open file ") + file.getFullPathName());
      return;
    }
    while (!is->isExhausted())
    {
      std::vector<double> example;
      StringArray tokens;
      tokens.addTokens(is->readNextLine(), T(" "));
      for (size_t i = 0; i < (size_t)tokens.size(); ++i)
        example.push_back(tokens[i].getDoubleValue());

      jassert((isClassification && example.size() == 22) || (!isClassification && example.size() == 11));
      results.push_back(example);
    }
    jassert(results.size() == 2300);
    delete is;
  }
  
  ContainerPtr loadDataToContainer(const std::vector<std::vector<double> >& data, size_t from, size_t to, bool isClassification)
  {
    if (data.empty())
      return ContainerPtr();
    jassert(from <= to && to <= data.size());
    TypePtr outputType = isClassification ? (TypePtr)waveFormTypeEnumeration : doubleType;
    ContainerPtr res = vector(pairClass(vectorClass(doubleType), outputType), to - from);
    for (size_t i = from; i < to; ++i)
    {
      ContainerPtr example = vector(doubleType, data[i].size());
      for (size_t j = 0; j < data[i].size(); ++j)
        example->setElement(j, Variable(data[i][j], doubleType));
      double output = data[i][data[i].size() - 1];
      if (isClassification)
        res->setElement(i - from, Variable::pair(example, Variable((int)output, outputType)));
      else
        res->setElement(i - from, Variable::pair(example, Variable(output, outputType)));
    }
    return res;
  }
};

};

#endif // 0

#endif // !LBCPP_TEST_UNIT_EXTRA_TREES_H_
