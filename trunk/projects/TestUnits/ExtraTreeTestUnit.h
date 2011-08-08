/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeTestUnit.h            | ExtraTree Test Unit             |
| Author  : Julien Becker                  |                                 |
| Started : 16/12/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_TEST_UNIT_EXTRA_TREES_H_
# define LBCPP_TEST_UNIT_EXTRA_TREES_H_

# include <lbcpp/Execution/TestUnit.h>
# include <lbcpp/Learning/DecisionTree.h>

namespace lbcpp
{

extern EnumerationPtr waveFormTypeEnumeration;
 
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
    context.enterScope(T("Classification"));

    File input(File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/ExtraTrees/classification.csv")));

    std::vector<std::vector<double> > data;
    parseDataFile(context, input, data, true);

    ContainerPtr learningData = loadDataToContainer(context, data, 0, 300, true);
    ContainerPtr testingData = loadDataToContainer(context, data, 300, 2300, true);

    FunctionPtr learner = classificationExtraTree(waveFormTypeEnumeration, numTrees, numAttributes, minSplitSize);

    learner->train(context, learningData, ContainerPtr(), T("Training"));
    
    EvaluatorPtr evaluator = classificationEvaluator();
    ScoreObjectPtr score = learner->evaluate(context, learningData, evaluator, T("Evaluating on training data"));
    checkIsCloseTo(context, 1.0, 0.0, 1 - score->getScoreToMinimize());

    evaluator = classificationEvaluator();
    score = learner->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));
    checkIsCloseTo(context, 0.85, 0.03, 1 - score->getScoreToMinimize());
    
    context.leaveScope();
  }

  void runRegression(ExecutionContext& context)
  {
    context.enterScope(T("Regression"));

    File input(File::getCurrentWorkingDirectory().getChildFile(T("../../projects/Examples/Data/ExtraTrees/regression.csv")));
    
    std::vector<std::vector<double> > data;
    parseDataFile(context, input, data, false);
    
    ContainerPtr learningData = loadDataToContainer(context, data, 0, 300, false);
    ContainerPtr testingData = loadDataToContainer(context, data, 300, 2300, false);
    
    FunctionPtr learner = regressionExtraTree(numTrees, numAttributes, minSplitSize);

    learner->train(context, learningData, ContainerPtr(), T("Training"));

    EvaluatorPtr evaluator = regressionEvaluator();
    ScoreObjectPtr score = learner->evaluate(context, learningData, evaluator, T("Evaluating on training data"));
    checkIsCloseTo(context, 0.0, 0.0001, score->getScoreToMinimize());

    evaluator = regressionEvaluator();
    score = learner->evaluate(context, testingData, evaluator, T("Evaluating on testing data"));
    checkIsCloseTo(context, 2.2, 0.3, score->getScoreToMinimize());
    
    context.leaveScope();
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
  
  ContainerPtr loadDataToContainer(ExecutionContext& context, const std::vector<std::vector<double> >& data, size_t from, size_t to, bool isClassification)
  {
    if (data.empty())
      return ContainerPtr();
    jassert(from <= to && to <= data.size());
    TypePtr outputType = isClassification ? (TypePtr)waveFormTypeEnumeration : doubleType;
    
    DefaultEnumerationPtr enumeration = new DefaultEnumeration();
    for (size_t i = 0; i < data[0].size(); ++i)
      enumeration->addElement(context, String((int)i));

    ContainerPtr res = vector(pairClass(doubleVectorClass(enumeration, doubleType), outputType), to - from);
    for (size_t i = from; i < to; ++i)
    {
      ContainerPtr example = new DenseDoubleVector(enumeration, doubleType);
      for (size_t j = 0; j < data[i].size() - 1; ++j)
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

#endif // !LBCPP_TEST_UNIT_EXTRA_TREES_H_
