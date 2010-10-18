// OnlineFQI


#include <lbcpp/lbcpp.h>
using namespace lbcpp;

extern void declareGlopClasses();

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

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
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
  declareGlopClasses();

  // create linear regressor
  PerceptionPtr perception = new XorExamplePerception();
  InferenceOnlineLearnerPtr learner = gradientDescentInferenceOnlineLearner(
          InferenceOnlineLearner::never, // randomization
          InferenceOnlineLearner::perStep, constantIterationFunction(0.1), true, // learning steps
          InferenceOnlineLearner::never, ScalarObjectFunctionPtr(), // regularizer
          InferenceOnlineLearner::perPass, maxIterationsStoppingCriterion(100), true);
  InferencePtr regressor = squareRegressionInference(perception, learner, T("toto"));
 
  // make training set
  TypePtr inputType = perception->getInputType();
  VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

  // create context and train
  InferenceContextPtr context = multiThreadedInferenceContext(8);
  context->train(regressor, trainingSet);

  // evaluate
  EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
  context->evaluate(regressor, trainingSet, evaluator);
  std::cout << "Evaluation: " << evaluator->toString() << std::endl;

  // test evaluator on one example
  Inference::ReturnCode returnCode = Inference::finishedReturnCode;
  Variable myPrediction = context->run(regressor, Variable::pair(1.0, 0.0), Variable(), returnCode);
  std::cout << "MyPrediction: " << myPrediction << std::endl;
  return 0;
}

