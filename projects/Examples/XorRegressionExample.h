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

class LearnableXorFunction : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input1 = builder.addInput(doubleType, T("x1"));
    size_t input2 = builder.addInput(doubleType, T("x2"));
    size_t supervision = builder.addInput(doubleType, T("supervision"));
    size_t features = builder.addFunction(new XorFeatureGenerator(), input1, input2, T("features"));

    StochasticGDParametersPtr params = new StochasticGDParameters();
    params->setEvaluator(oldRegressionErrorEvaluator(T("xor-error")));
    builder.addFunction(linearLearningMachine(params), features, supervision);
  }
};

class XorRegressionExample : public WorkUnit
{
public:
  FunctionPtr createXorFunction(ExecutionContext& context)
  {
    FunctionPtr res = new LearnableXorFunction();
    return res->initialize(context, std::vector<TypePtr>(3, doubleType)) ? res : FunctionPtr();
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
    OldEvaluatorPtr evaluator = oldRegressionErrorEvaluator(T("XOR-error"));
    xorFunction->evaluate(context, trainingExamples, evaluator);
    //std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    Variable myPrediction = xorFunction->compute(context, 1.0, 0.0, Variable::missingValue(doubleType));
    context.resultCallback(T("prediction"), myPrediction);

    context.checkSharedPointerCycles(xorFunction);
    context.checkSharedPointerCycles(refCountedPointerFromThis(this));
    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_XOR_REGRESSION_H_
