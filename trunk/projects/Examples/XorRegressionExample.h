/*-----------------------------------------.---------------------------------.
| Filename: XorRegressionExample.h         | Illustrates a simple regression |
| Author  : Francis Maes                   |                                 |
| Started : 18/10/2010 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_XOR_REGRESSION_H_
# define LBCPP_EXAMPLES_XOR_REGRESSION_H_

# include <lbcpp/Core/Frame.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class XorFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleType;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("xor-features"));
    res->addElement(context, T("unit"));
    res->addElement(context, T("x1"));
    res->addElement(context, T("x2"));
    res->addElement(context, T("x1.x2"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    double x1 = inputs[0].getDouble();
    double x2 = inputs[1].getDouble();
    callback.sense(0, 1.0);
    callback.sense(1, x1);
    callback.sense(2, x2);
    callback.sense(3, x1 * x2);
  }
};

class XorRegressionExample : public WorkUnit
{
public:
  FrameClassPtr createXorFrameClass(ExecutionContext& context)
  {
    FrameClassPtr res = new FrameClass(T("XorFrame"));
    res->addMemberVariable(context, doubleType, T("x1"));
    res->addMemberVariable(context, doubleType, T("x2"));
    res->addMemberVariable(context, doubleType, T("supervision"));
    res->addMemberOperator(context, new XorFeatureGenerator(), 0, 1, T("features"));
    res->addMemberOperator(context, linearRegressor(), 3, 2);
    return res;
  }

  FunctionPtr createXorFunction(ExecutionContext& context, FrameClassPtr xorFrameClass)
  {
    FunctionPtr xorFunction = new FrameBasedFunction(xorFrameClass);
    xorFunction->setBatchLearner(frameBasedFunctionBatchLearner());
    xorFunction->initialize(context, std::vector<TypePtr>(3, doubleType));
    return xorFunction;
  }

  VectorPtr createTrainingExamples(FrameClassPtr frameClass) const
  {
    VectorPtr trainingExamples = vector(frameClass);
    trainingExamples->append(new Frame(frameClass, 0.0, 0.0, 1.0));
    trainingExamples->append(new Frame(frameClass, 1.0, 0.0, 0.0));
    trainingExamples->append(new Frame(frameClass, 0.0, 1.0, 0.0));
    trainingExamples->append(new Frame(frameClass, 1.0, 1.0, 1.0));
    return trainingExamples;
  }

  virtual Variable run(ExecutionContext& context)
  {
    FrameClassPtr xorFrameClass = createXorFrameClass(context);
    VectorPtr trainingExamples = createTrainingExamples(xorFrameClass);
    
    FunctionPtr xorFunction = createXorFunction(context, xorFrameClass);
    if (!xorFunction->train(context, trainingExamples))
      return false;

    // evaluate
    EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
    xorFunction->evaluate(context, trainingExamples, evaluator);
    //std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    Variable myPrediction = xorFunction->compute(context, 1.0, 0.0, Variable::missingValue(doubleType));
    context.resultCallback(T("prediction"), myPrediction);

    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_XOR_REGRESSION_H_
