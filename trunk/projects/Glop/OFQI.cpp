#include <lbcpp/lbcpp.h>
#include "XorExamplePerception.h"
#include "OFQIInference.h"

using namespace lbcpp;

extern void declareGlopClasses(ExecutionContext& context);

class MyInferenceCallback : public InferenceCallback
{
public:
  virtual void preInferenceCallback(ExecutionContext& context, const InferenceStackPtr& stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (input.size() == 2 && input[0].getType()->inheritsFrom(inferenceClass))
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      context.informationCallback(T("=== Learning ") + input[0].getObject()->getName() + T(" with ") + String((int)input[1].size()) + T(" ") + inputTypeName + T("(s) ==="));
      //std::cout << "  learner: " << inferenceClassName << " static type: " << input[1].getTypeName() << std::endl
      //  << "  first example type: " << input[1][0].getTypeName() << std::endl << std::endl;
    }
  }

  virtual void postInferenceCallback(ExecutionContext& context, const InferenceStackPtr& stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
  }
};


int main(int argc, char* argv[])
{
  lbcpp::initialize();
  ExecutionContextPtr context = defaultConsoleExecutionContext();
  declareGlopClasses(*context);

  // create linear regressor
  PerceptionPtr perception = new XorExamplePerception();
  InferenceOnlineLearnerPtr learner = gradientDescentOnlineLearner(
          perStep, constantIterationFunction(0.1), true, // learning steps
          never, ScalarObjectFunctionPtr()); // regularizer
  learner->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(2), true));
  NumericalSupervisedInferencePtr regressor = squareRegressionInference(T("toto regr"), perception);
  regressor->setStochasticLearner(learner);

  InferencePtr ofqiInference = new OFQIInference(*context, 5, 0.9, regressor);
  ofqiInference->setBatchLearner(stochasticInferenceLearner());
 
  // make training set
  TypePtr inputType = perception->getInputType();
  VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
  trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

  // create context and train
  context->appendCallback(new MyInferenceCallback());
  train(*context, ofqiInference, trainingSet, ContainerPtr());

  // evaluate
  EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
  evaluate(*context, ofqiInference, trainingSet, evaluator);
  std::cout << "Evaluation: " << evaluator->toString() << std::endl;

  // test evaluator on one example
  Inference::ReturnCode returnCode = Inference::finishedReturnCode;
  Variable myPrediction = predict(*context, regressor, Variable::pair(1.0, 0.0));
  std::cout << "MyPrediction: " << myPrediction << std::endl;
  return 0;
}

