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
  FunctionPtr createXorFunction(ExecutionContext& context)
  {
    FrameClassPtr frameClass = new FrameClass(T("XorFrame"));
    frameClass->addMemberVariable(context, doubleType, T("x1"));
    frameClass->addMemberVariable(context, doubleType, T("x2"));
    frameClass->addMemberVariable(context, doubleType, T("supervision"));
    frameClass->addMemberOperator(context, new XorFeatureGenerator(), 0, 1, T("features"));
    frameClass->addMemberOperator(context, linearRegressor(), 3, 2);

    FunctionPtr xorFunction = new FrameBasedFunction(frameClass);
    xorFunction->setBatchLearner(frameBasedFunctionBatchLearner());
    xorFunction->initialize(context, std::vector<TypePtr>(3, doubleType));
    return xorFunction;
  }

  VectorPtr createTrainingExamples(ClassPtr inputsClass) const
  {
    VectorPtr trainingExamples = vector(inputsClass);
    trainingExamples->append(new DenseGenericObject(inputsClass, 0.0, 0.0, 1.0));
    trainingExamples->append(new DenseGenericObject(inputsClass, 1.0, 0.0, 0.0));
    trainingExamples->append(new DenseGenericObject(inputsClass, 0.0, 1.0, 0.0));
    trainingExamples->append(new DenseGenericObject(inputsClass, 1.0, 1.0, 1.0));
    return trainingExamples;
  }

  virtual Variable run(ExecutionContext& context)
  {
    FunctionPtr xorFunction = createXorFunction(context);
    
    VectorPtr trainingExamples = createTrainingExamples(xorFunction->getInputsClass());
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
