/*-----------------------------------------.---------------------------------.
| Filename: XorRegressionExample.h         | Illustrates a simple regression |
| Author  : Francis Maes                   |                                 |
| Started : 18/10/2010 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_XOR_REGRESSION_H_
# define LBCPP_EXAMPLES_XOR_REGRESSION_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include <lbcpp/Core/Frame.h>

namespace lbcpp
{

class XorFeatureGenerator : public FeatureGenerator
{
public:
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!checkNumInputs(context, 1) || !checkInputType(context, 0, pairClass(doubleType, doubleType)))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("xor-features"));
    res->addElement(context, T("unit"));
    res->addElement(context, T("x1"));
    res->addElement(context, T("x2"));
    res->addElement(context, T("x1.x2"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const PairPtr& pair = inputs->getObjectAndCast<Pair>();
    double x1 = pair->getFirst().getDouble();
    double x2 = pair->getSecond().getDouble();
    callback.sense(0, 1.0);
    callback.sense(1, x1);
    callback.sense(2, x2);
    callback.sense(3, x1 * x2);
  }
};

///////////////////////////////////////////////////////////////

// Function, Container[VariableVector], optional Container[VariableVector] -> Nil
// LearnableFunction, Container[VariableVector], optional Container[VariableVector] -> Parameters
class BatchLearner : public Function
{
public:
  
};

typedef ReferenceCountedObjectPtr<BatchLearner> BatchLearnerPtr;

class OnlineLearner : public Function
{
public:
  virtual void startLearningCallback(ExecutionContext& context);
  virtual void subStepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction);
  virtual void episodeFinishedCallback(ExecutionContext& context, const InferencePtr& inference);
  // batchLearnerInput: if learning is performed within the context of a batch learner, we have batch learning parameters here
  virtual void passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput);
};

typedef ReferenceCountedObjectPtr<OnlineLearner> OnlineLearnerPtr;

///////////////////////////////////////////////////////////////

class LearnableFunction : public Function
{
public:
  ObjectPtr getParameters() const
    {return parameters;}

protected:
  friend class LearnableFunctionClass;

  BatchLearnerPtr batchLearner;
  OnlineLearnerPtr onlineLearner;

  CriticalSection parametersLock;
  ObjectPtr parameters;
  ClassPtr parametersClass;
};

class NumericalLearnableFunction : public LearnableFunction
{
public:
  DoubleVectorPtr getParameters() const
    {return parameters.staticCast<DoubleVector>();}
};

// DoubleVector<T>, optional LossFunction -> Double
class LearnableDotProductFunction : public NumericalLearnableFunction
{
public:
  DenseDoubleVectorPtr getParameters() const
    {return parameters.staticCast<DenseDoubleVector>();}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    EnumerationPtr featuresEnumeration;
    TypePtr featuresType;
    if (!checkNumInputs(context, 2) ||
        !DoubleVector::getTemplateParameters(context, getInputType(0), featuresEnumeration, featuresType) ||
        !checkInputType(context, 1, functionClass))
      return VariableSignaturePtr();

    parametersClass = denseDoubleVectorClass(featuresEnumeration, featuresType);
    if (!parameters)
      parameters = new DenseDoubleVector(parametersClass);
    return new VariableSignature(doubleType, T("prediction"), T("p"));
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    DoubleVectorPtr inputVector = inputs[0].getObjectAndCast<DoubleVector>();
    //FunctionPtr supervision = inputs[1].getObjectAndCast<Function>();
    DenseDoubleVectorPtr parameters = getParameters();
    if (!parameters || !inputVector)
      return Variable::missingValue(doubleType);

    double res = inputVector->dotProduct(parameters);
    return isNumberValid(res) ? Variable(res) : Variable::missingValue(doubleType);
  }
};
/*
class ApplyOnSingleVariableFunction : public Function
{
public:
  ApplyOnSingleVariableFunction(size_t inputIndex, FunctionPtr function)
    : inputIndex(inputIndex), function(function) {}
 
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (getNumInputs() <= inputIndex)
    {
      context.errorCallback(T("Not enough inputs"));
      return VariableSignaturePtr();
    }
    if (!function->initialize(context, getInputVariable(inputIndex))
      return VariableSignaturePtr();
    return new VariableSignaturePtr(
 

protected:
  size_t inputIndex;
  FunctionPtr function;
};*/

class MakeRegressionLossFunction : public Function
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1) || !checkInputType(context, 0, doubleType))
      return VariableSignaturePtr();
    return new VariableSignature(scalarFunctionClass, T("loss"), T("l"));
  }
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return squareLossFunction(inputs[0].getDouble());}
};

class LinearRegressor : public FrameBasedFunction
{
public:
  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    EnumerationPtr featuresEnumeration;
    TypePtr featuresType;
    if (!checkNumInputs(context, 2) ||
        !DoubleVector::getTemplateParameters(context, getInputType(0), featuresEnumeration, featuresType) ||
        !checkInputType(context, 1, doubleType))
      return VariableSignaturePtr();

    frameClass = new FrameClass(T("LinearRegressor"));
    frameClass->addMemberVariable(context, getInputType(0), T("input"));
    frameClass->addMemberVariable(context, getInputType(1), T("supervision"));
    frameClass->addMemberOperator(context, new MakeRegressionLossFunction(), 1);
    frameClass->addMemberOperator(context, new LearnableDotProductFunction(), 0, 2);
    return FrameBasedFunction::initializeFunction(context);
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
    res->addMemberOperator(context, new LinearRegressor(), 3, 2);
    return res;
  }

  VectorPtr createTrainingExamples(FrameClassPtr frameClass) const
  {
    VectorPtr trainingExamples = vector(frameClass);
    
    FramePtr example = new Frame(frameClass);
    example->setVariable(0, 0.0);
    example->setVariable(1, 0.0);
    example->setVariable(2, 1.0);
    trainingExamples->append(example);

    example = new Frame(frameClass);
    example->setVariable(0, 1.0);
    example->setVariable(1, 0.0);
    example->setVariable(2, 0.0);
    trainingExamples->append(example);

    example = new Frame(frameClass);
    example->setVariable(0, 0.0);
    example->setVariable(1, 1.0);
    example->setVariable(2, 0.0);
    trainingExamples->append(example);

    example = new Frame(frameClass);
    example->setVariable(0, 1.0);
    example->setVariable(1, 1.0);
    example->setVariable(2, 1.0);
    trainingExamples->append(example);

    return trainingExamples;
  }

  virtual Variable run(ExecutionContext& context)
  {
    FrameClassPtr frameClass = createXorFrameClass(context);
    VectorPtr trainingExamples = createTrainingExamples(frameClass);
    
    // todo: learn and evaluate

#if 0
   // create linear regressor
    InferenceOnlineLearnerPtr learner = gradientDescentOnlineLearner(
            perStep, constantIterationFunction(0.1), true, // learning steps
            never, ScalarObjectFunctionPtr()); // regularizer
    learner->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(100), true)); // stopping criterion
    NumericalSupervisedInferencePtr regressor = squareRegressionInference(T("XOR-Regressor"), PerceptionPtr());
    regressor->setStochasticLearner(learner);

    TypePtr inputType = pairClass(doubleType, doubleType);

    FeatureGeneratorPtr featureGenerator = new XorFeatureGenerator();
    featureGenerator->initialize(context, inputType);
    InferencePtr inference = preProcessInference(regressor, featureGenerator);

    // make training set
    VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
    
    trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

    // create context and train
    inference->train(context, trainingSet, ContainerPtr(), T("Training"));

    // evaluate
    EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
    inference->evaluate(context, trainingSet, evaluator, T("Evaluating"));
    //std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    // test evaluator on one example
    Variable myPrediction = inference->computeFunction(context, Variable::pair(1.0, 0.0));
    //std::cout << "MyPrediction: " << myPrediction << std::endl;
#endif // 0

    FramePtr myFrame = new Frame(frameClass);
    myFrame->setVariable(0, 1.0);
    myFrame->setVariable(1, 0.0);
    Variable myPrediction = myFrame->getVariable(4);
    context.resultCallback(T("prediction"), myPrediction);

    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_XOR_REGRESSION_H_
