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

namespace lbcpp
{

class ProductFunction : public SimpleBinaryFunction
{
public:
  ProductFunction() : SimpleBinaryFunction(doubleType, doubleType, doubleType, "prod") {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[0].toDouble() * inputs[1].toDouble();}
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
    : SimpleUnaryFunction(doubleType, booleanType, "stump"), threshold(threshold) {}

  virtual String toShortString() const
    {return T("Stump(") + String(threshold) + T(")");}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.toDouble() >= threshold;}

protected:
  friend class StumpFunctionClass;

  double threshold;
};

class LuapeSandBox : public WorkUnit
{
public:
  LuapeSandBox() : maxExamples(0), maxSteps(3), maxIterations(10) {}

  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr features = new DefaultEnumeration("features");
    ContainerPtr trainData = loadData(context, trainFile, features);
    ContainerPtr testData = loadData(context, testFile, features);
    if (!trainData || !testData)
      return false;

    context.informationCallback(
      String((int)trainData->getNumElements()) + T(" training examples, ") +
      String((int)testData->getNumElements()) + T(" testing examples, ") + 
      String((int)features->getNumElements()) + T(" features"));

    LuapeProblemPtr problem = createProblem(features);

    LuapeFunctionPtr classifier = new LuapeBinaryClassifier();
    classifier->setBatchLearner(new AdaBoostLuapeLearner(problem, maxSteps, maxIterations));
    classifier->setEvaluator(binaryClassificationEvaluator());

    classifier->train(context, trainData, testData, T("Training"), true);
    classifier->evaluate(context, trainData, EvaluatorPtr(), T("Evaluating on training data"));
    classifier->evaluate(context, testData, EvaluatorPtr(), T("Evaluating on testing data"));
    return true;
  }

protected:
  friend class LuapeSandBoxClass;

  File trainFile;
  File testFile;
  size_t maxExamples;
  size_t maxSteps;
  size_t maxIterations;

  ContainerPtr loadData(ExecutionContext& context, const File& file, DefaultEnumerationPtr features) const
  {
    context.enterScope(T("Loading ") + file.getFileName());
    ContainerPtr res = binaryClassificationLibSVMFastParser(context, file, features)->load(maxExamples);
    if (res && !res->getNumElements())
      res = ContainerPtr();
    context.leaveScope(res ? res->getNumElements() : 0);
    return res;
  }

  LuapeProblemPtr createProblem(const EnumerationPtr& features)
  {
    LuapeProblemPtr res = new LuapeProblem();
    size_t n = features->getNumElements();
    for (size_t i = 0; i < n; ++i)
      res->addInput(doubleType, features->getElementName(i));

    res->addFunction(new LogFunction());
    //res->addFunction(new ProductFunction());
    for (double k = -1.0; k <= 1.0; k += 0.2)
      res->addFunction(new StumpFunction(k));
    res->addFunction(new BooleanAndFunction());
    res->addFunction(new GreaterThanFunction());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_SAND_BOX_H_
