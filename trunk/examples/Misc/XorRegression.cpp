/*-----------------------------------------.---------------------------------.
| Filename: XorRegression.cpp              | Illustrates a simple regression |
| Author  : Francis Maes                   |                                 |
| Started : 18/10/2010 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareGlopClasses(ExecutionContext& context);

class XorExamplePerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairClass(doubleType, doubleType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("unit"), doubleType);
    addOutputVariable(T("x1"), doubleType);
    addOutputVariable(T("x2"), doubleType);
    addOutputVariable(T("x1.x2"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    double x1 = input[0].getDouble();
    double x2 = input[1].getDouble();
    callback->sense(0, 1.0);
    callback->sense(1, x1);
    callback->sense(2, x2);
    callback->sense(3, x1 * x2);
  }
};

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  ExecutionContextPtr context = defaultConsoleExecutionContext();
  context->declareType(TypePtr(new DefaultClass(T("XorExamplePerception"), T("Perception"))));
 
  // create linear regressor
  PerceptionPtr perception = new XorExamplePerception();
  InferenceOnlineLearnerPtr learner = gradientDescentOnlineLearner(
          perStep, constantIterationFunction(0.1), true, // learning steps
          never, ScalarObjectFunctionPtr()); // regularizer
  learner->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(100), true)); // stopping criterion
  NumericalSupervisedInferencePtr regressor = squareRegressionInference(T("XOR-Regressor"), perception);
  regressor->setStochasticLearner(learner);
 
  // make training set
  TypePtr inputType = perception->getInputType();
  VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

  // create context and train
  train(*context, regressor, trainingSet, ContainerPtr());

  // evaluate
  EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
  evaluate(*context, regressor, trainingSet, evaluator);
  std::cout << "Evaluation: " << evaluator->toString() << std::endl;

  // test evaluator on one example
  Inference::ReturnCode returnCode = Inference::finishedReturnCode;
  Variable myPrediction = predict(*context, regressor, Variable::pair(1.0, 0.0));
  std::cout << "MyPrediction: " << myPrediction << std::endl;
  return 0;
}

