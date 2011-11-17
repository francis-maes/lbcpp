/*-----------------------------------------.---------------------------------.
| Filename: LuapeSandBox.h                 | Luape Sand Box                  |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 11:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_SAND_BOX_H_
# define LBCPP_LUAPE_SAND_BOX_H_

# include <lbcpp/Data/Stream.h>
# include "LuapeProblem.h"
# include "LuapeBatchLearner.h"
# include "../Core/SinglePlayerMCTSOptimizer.h"

namespace lbcpp
{

class AddFunction : public SimpleBinaryFunction
{
public:
  AddFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "add") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() + inputs[1].toDouble();}
};

class SubFunction : public SimpleBinaryFunction
{
public:
  SubFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "sub") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() - inputs[1].toDouble();}
};

class ProductFunction : public SimpleBinaryFunction
{
public:
  ProductFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "prod") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() * inputs[1].toDouble();}
};

class DivideFunction : public SimpleBinaryFunction
{
public:
  DivideFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "div") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    double a = inputs[0].toDouble();
    double b = inputs[1].toDouble();
    return b ? a / b : DBL_MAX;
  }
};

class GreaterThanFunction : public SimpleBinaryFunction
{
public:
  GreaterThanFunction() : SimpleBinaryFunction(doubleType, doubleType, booleanType, "gt") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() > inputs[1].toDouble();}
};

class BooleanAndFunction : public SimpleBinaryFunction
{
public:
  BooleanAndFunction() : SimpleBinaryFunction(booleanType, booleanType, booleanType, "and") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].getBoolean() && inputs[1].getBoolean();}
};

class BooleanXorFunction : public SimpleBinaryFunction
{
public:
  BooleanXorFunction() : SimpleBinaryFunction(booleanType, booleanType, booleanType, "xor") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].getBoolean() != inputs[1].getBoolean();}
};

class LogFunction : public SimpleUnaryFunction
{
public:
  LogFunction() : SimpleUnaryFunction(doubleType, doubleType, "log") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return log(input.toDouble());}
};

class StumpFunction : public SimpleUnaryFunction
{
public:
  StumpFunction(double threshold = 0.0) 
    : SimpleUnaryFunction(sumType(integerType, doubleType), booleanType, "stump"), threshold(threshold) {}

  virtual String toShortString() const
    {return T("Stump(") + String(threshold) + T(")");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.toDouble() >= threshold;}

protected:
  friend class StumpFunctionClass;

  double threshold;
};

class EqualsEnumValueFunction : public SimpleUnaryFunction
{
public:
  EqualsEnumValueFunction(const Variable& value = Variable()) 
    : SimpleUnaryFunction(enumValueType, booleanType, "equalsEnum"), value(value) {}

  virtual String toShortString() const
    {return T("Equals(") + value.toShortString() + T(")");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input == value;}

protected:
  friend class EqualsEnumValueFunctionClass;

  Variable value;
};


class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxExamples(0), maxSteps(3), budgetPerIteration(1000), maxIterations(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    DynamicClassPtr inputClass = new DynamicClass("inputs");
    DefaultEnumerationPtr labels = new DefaultEnumeration("labels");
    ContainerPtr trainData = loadData(context, trainFile, inputClass, labels);
    ContainerPtr testData = loadData(context, testFile, inputClass, labels);
    if (!trainData || !testData)
      return false;

    context.resultCallback("train", trainData);
    context.resultCallback("test", testData);

    context.informationCallback(
      String((int)trainData->getNumElements()) + T(" training examples, ") +
      String((int)testData->getNumElements()) + T(" testing examples, ") + 
      String((int)inputClass->getNumMemberVariables()) + T(" input variables,") +
      String((int)labels->getNumElements()) + T(" labels"));

    LuapeProblemPtr problem = createProblem(inputClass);

    LuapeFunctionPtr classifier = new LuapeClassifier();
    if (!classifier->initialize(context, inputClass, labels))
      return false;

    //OptimizerPtr optimizer = new NestedMonteCarloOptimizer(2, 1);

    OptimizerPtr optimizer = new SinglePlayerMCTSOptimizer(budgetPerIteration);
  
    LuapeWeakLearnerPtr weakLearner = combinedStumpWeakLearner();
    //productWeakLearner(singleStumpWeakLearner(), 2);
      //luapeGraphBuilderWeakLearner(optimizer, maxSteps);
    classifier->setBatchLearner(adaBoostMHLuapeLearner(problem, weakLearner, maxIterations));
    classifier->setEvaluator(defaultSupervisedEvaluator());

    classifier->train(context, trainData, testData, T("Training"), true);
    //classifier->evaluate(context, trainData, EvaluatorPtr(), T("Evaluating on training data"));
    //classifier->evaluate(context, testData, EvaluatorPtr(), T("Evaluating on testing data"));
    return true;
  }

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxExamples;
  size_t maxSteps;
  size_t budgetPerIteration;
  size_t maxIterations;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr labels) const
  {
    context.enterScope(T("Loading ") + file.getFileName());
    ContainerPtr res = classificationARFFDataParser(context, file, inputClass, labels)->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  LuapeProblemPtr createProblem(DynamicClassPtr inputClass)
  {
    LuapeProblemPtr res = new LuapeProblem();
    size_t n = inputClass->getNumMemberVariables();
    for (size_t i = 0; i < n; ++i)
    {
      VariableSignaturePtr variable = inputClass->getMemberVariable(i);
      res->addInput(variable->getType(), variable->getName());
    }

//    res->addFunction(new LogFunction());
    res->addFunction(new DivideFunction());
//    res->addFunction(new ProductFunction());
    res->addFunction(new StumpFunction());
    res->addFunction(new BooleanAndFunction());
//    res->addFunction(new GreaterThanFunction());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SAND_BOX_H_
